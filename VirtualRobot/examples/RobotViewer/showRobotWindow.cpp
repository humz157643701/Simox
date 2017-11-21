
#include "showRobotWindow.h"
#include "../../EndEffector/EndEffector.h"
#include "../../Tools/RuntimeEnvironment.h"
#include "../../Import/RobotImporterFactory.h"
#include "../../Model/Nodes/ModelJoint.h"
#include "../../Model/LinkSet.h"
#include "../../Model/JointSet.h"
#include "../../Import/SimoxXMLFactory.h"
#include "../../Visualization/CoinVisualization/CoinVisualizationFactory.h"
#include "../../Model/Nodes/Attachments/ModelNodeAttachmentFactory.h"

#include <QFileDialog>
#include <Eigen/Geometry>

#include <boost/algorithm/string/predicate.hpp>
#include <time.h>
#include <vector>
#include <iostream>
#include <cmath>

#include <sstream>

#ifdef Simox_USE_COIN_VISUALIZATION
    #include "../../../Gui/Coin/CoinViewerFactory.h"
    // TODO get rid of this hack.
    // need this to ensure that static Factory methods are called across library boundaries (otherwise coin Gui lib is not loaded since it is not referenced by us)
    SimoxGui::CoinViewerFactory f;
#endif

using namespace std;
using namespace VirtualRobot;

showRobotWindow::showRobotWindow(std::string& sRobotFilename)
    : QMainWindow(NULL)
{
    useColModel = false;
    VirtualRobot::RuntimeEnvironment::getDataFileAbsolute(sRobotFilename);
    robotFilename = sRobotFilename;

    loadRobot();
    setupUI();

    viewer->viewAll();
}


showRobotWindow::~showRobotWindow()
{
    robot.reset();
}

void showRobotWindow::setupUI()
{
    UI.setupUi(this);

    SimoxGui::ViewerFactoryPtr viewerFactory = SimoxGui::ViewerFactory::first(NULL);
    THROW_VR_EXCEPTION_IF(!viewerFactory,"No viewer factory?!");
    viewer = viewerFactory->createViewer(UI.frameViewer);

    // joint sets
    UI.cBoxJointSets->clear();
    UI.cBoxJointSets->addItem("All");
    for (auto & js : robot->getJointSets())
    {
        UI.cBoxJointSets->addItem(QString::fromStdString(js->getName()));
    }
    // link sets
    UI.cBoxLinkSets->clear();
    UI.cBoxLinkSets->addItem("All");
    for (auto & ls : robot->getLinkSets())
    {
        UI.cBoxLinkSets->addItem(QString::fromStdString(ls->getName()));
    }

    updateModelNodeControls();

    connect(UI.btnLoadRobot, SIGNAL(clicked()), this, SLOT(selectRobot()));
    connect(UI.btnResetRobot, SIGNAL(clicked()), this, SLOT(resetRobot()));

    connect(UI.pushButtonClose, SIGNAL(clicked()), this, SLOT(closeHand()));
    connect(UI.ExportVRML20, SIGNAL(clicked()), this, SLOT(exportVRML()));
    connect(UI.ExportXML, SIGNAL(clicked()), this, SLOT(exportXML()));
    connect(UI.pushButtonOpen, SIGNAL(clicked()), this, SLOT(openHand()));
    connect(UI.comboBoxEndEffector, SIGNAL(activated(int)), this, SLOT(selectEEF(int)));
    connect(UI.comboBoxEndEffectorPS, SIGNAL(activated(int)), this, SLOT(selectPreshape(int)));

    connect(UI.checkBoxPhysicsCoM, SIGNAL(clicked()), this, SLOT(displayPhysics()));
    connect(UI.checkBoxPhysicsInertia, SIGNAL(clicked()), this, SLOT(displayPhysics()));

    connect(UI.checkBoxColModel, SIGNAL(clicked()), this, SLOT(rebuildVisualization()));
    connect(UI.checkBoxRobotSensors, SIGNAL(clicked()), this, SLOT(showSensors()));
    connect(UI.checkBoxStructure, SIGNAL(clicked()), this, SLOT(robotStructure()));
    UI.checkBoxFullModel->setChecked(true);
    connect(UI.checkBoxFullModel, SIGNAL(clicked()), this, SLOT(robotFullModel()));
    connect(UI.checkBoxRobotCoordSystems, SIGNAL(clicked()), this, SLOT(robotCoordSystems()));

    connect(UI.cBoxJointSets, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModelNodeControls()));
    connect(UI.cBoxLinkSets, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModelNodeControls()));

    rebuildVisualization();
}

void showRobotWindow::resetRobot()
{
    if (!robot) return;

    std::map<std::string, float> jv;
    for (auto& j: robot->getJoints())
    {
        jv[j->getName()] = 0.0f;
    }
    robot->setJointValues(jv);

    updateModelNodeControls();
}

void showRobotWindow::displayTriangles()
{
    QString text1, text2, text3;
    int trisAllFull, trisRNSFull, trisJointFull;
    trisAllFull = trisRNSFull = trisJointFull = 0;
    int trisAllCol, trisRNSCol, trisJointCol;
    trisAllCol = trisRNSCol = trisJointCol = 0;

    if (robot)
    {
        trisAllFull = robot->getNumFaces(false);
        trisAllCol = robot->getNumFaces(true);
        trisRNSFull = trisAllFull;
        trisRNSCol = trisAllCol;
    }

    LinkSetPtr ls = dynamic_pointer_cast<LinkSet>(currentRobotNodeSet);
    if (ls)
    {
        trisRNSFull = ls->getNumFaces(false);
        trisRNSCol = ls->getNumFaces(true);
    }

    ModelLinkPtr ml = dynamic_pointer_cast<ModelLink>(currentRobotNode);
    if (ml)
    {
        trisJointFull = ml->getNumFaces(false);
        trisJointCol = ml->getNumFaces(true);
    }

    if (UI.checkBoxColModel->isChecked())
    {
        text1 = tr("Total\t:") + QString::number(trisAllCol);
        text2 = tr("RobotNodeSet:\t") + QString::number(trisRNSCol);
        text3 = tr("Joint:\t") + QString::number(trisJointCol);
    }
    else
    {
        text1 = tr("Total:\t") + QString::number(trisAllFull);
        text2 = tr("RobotNodeSet:\t") + QString::number(trisRNSFull);
        text3 = tr("Joint:\t") + QString::number(trisJointFull);
    }

    UI.labelInfo1->setText(text1);
    UI.labelInfo2->setText(text2);
    UI.labelInfo3->setText(text3);
}

void showRobotWindow::robotFullModel()
{
    if (!robot)
    {
        return;
    }

    bool showFullModel = UI.checkBoxFullModel->isChecked();

    robot->setupVisualization(showFullModel, true);
    rebuildVisualization();
}

void showRobotWindow::rebuildVisualization()
{
    if (!robot)
    {
        return;
    }

    viewer->clearLayer("robotLayer");

    useColModel = UI.checkBoxColModel->isChecked();
    //bool sensors = UI.checkBoxRobotSensors->checkState() == Qt::Checked;
    ModelLink::VisualizationType colModel = (UI.checkBoxColModel->isChecked()) ? ModelLink::VisualizationType::Collision : ModelLink::VisualizationType::Full;

    VisualizationPtr visu = VisualizationFactory::getGlobalVisualizationFactory()->getVisualization(robot, colModel);
    viewer->addVisualization("robotLayer", "robot", visu);


    UI.checkBoxStructure->setEnabled(!useColModel);
    UI.checkBoxRobotSensors->setEnabled(!useColModel);
    UI.checkBoxFullModel->setEnabled(!useColModel);
    UI.checkBoxRobotCoordSystems->setEnabled(!useColModel);

}

void showRobotWindow::showSensors()
{
    if (!robot)
    {
        return;
    }

    bool showSensors = UI.checkBoxRobotSensors->isChecked();

    //todo
    /*

    std::vector<SensorPtr> sensors = robot->getSensors();

    for (size_t i = 0; i < sensors.size(); i++)
    {
        sensors[i]->setupVisualization(showSensors, showSensors);
        sensors[i]->showCoordinateSystem(showSensors);
    }
    */

    // rebuild visualization
    rebuildVisualization();
}



void showRobotWindow::displayPhysics()
{
    if (!robot)
    {
        return;
    }

    physicsCoMEnabled = UI.checkBoxPhysicsCoM->checkState() == Qt::Checked;
    physicsInertiaEnabled = UI.checkBoxPhysicsInertia->checkState() == Qt::Checked;

    //todo
    /*
    robot->showPhysicsInformation(physicsCoMEnabled, physicsInertiaEnabled);
    */

    // rebuild visualization
    rebuildVisualization();

}

void showRobotWindow::exportVRML()
{
    if (!robot)
    {
        return;
    }


    // todo: remove?

    // VRML
    /*
    QString fi = QFileDialog::getSaveFileName(this, tr("VRML 2.0 File"), QString(), tr("VRML Files (*.wrl)"));
    std::string s = std::string(fi.toLatin1());

    if (!s.empty())
    {
        if (!boost::algorithm::ends_with(s, ".wrl"))
            s += ".wrl";
        ModelLink::VisualizationType colModel = (UI.checkBoxColModel->isChecked()) ? ModelLink::VisualizationType::Collision : ModelLink::VisualizationType::Full;
        visualization = robot->getVisualization<CoinVisualization>(colModel);
        visualization->exportToVRML2(s);
    }
    */
}


void showRobotWindow::exportXML()
{
    if (!robot)
    {
        return;
    }

    // XML
    QString fi = QFileDialog::getSaveFileName(this, tr("xml File"), QString(), tr("xml Files (*.xml)"));
    std::string s = std::string(fi.toLatin1());

    if (!s.empty())
    {

        boost::filesystem::path p1(s);
        std::string fn = p1.filename().generic_string();
        std::string fnPath = p1.parent_path().generic_string();
        SimoxXMLFactory::saveXML(robot, fn, fnPath);
    }
}


void showRobotWindow::closeEvent(QCloseEvent* event)
{
    quit();
    QMainWindow::closeEvent(event);
}


int showRobotWindow::main()
{
    viewer->start(this);
    return 0;
}


void showRobotWindow::quit()
{
    std::cout << "ShowRobotWindow: Closing" << std::endl;
    this->close();
    viewer->stop();
}

void showRobotWindow::selectRobot()
{
    string supportedExtensions = RobotImporterFactory::getAllExtensions();
    string supported = "Supported Formats, " + supportedExtensions + " (" + supportedExtensions + ")";
    string filter = supported + ";;" + RobotImporterFactory::getAllFileFilters();
    QString fi = QFileDialog::getOpenFileName(this, tr("Open Robot File"), QString(), tr(filter.c_str()));
    std::string s = robotFilename = std::string(fi.toLatin1());

    if (!s.empty())
    {
        robotFilename = s;
        loadRobot();
    }
}



void showRobotWindow::loadRobot()
{
    VR_INFO << "Loading Robot from " << robotFilename << endl;
    currentEEF.reset();
    currentRobotNode.reset();
    currentNodes.clear();
    currentRobotNodeSet.reset();
    robot.reset();

    try
    {
        robot = ModelIO::loadModel(robotFilename, ModelIO::eFull);
    }
    catch (VirtualRobotException& /*e*/)
    {
    }

    if (!robot)
    {
        try
        {
            QFileInfo fileInfo(robotFilename.c_str());
            std::string suffix(fileInfo.suffix().toLatin1());
            RobotImporterFactoryPtr importer = RobotImporterFactory::fromFileExtension(suffix, NULL);

            if (!importer)
            {
                VR_WARNING << " ERROR while grabbing importer" << endl;
                return;
            }

            robot = importer->loadFromFile(robotFilename, ModelIO::eFull);
        }
        catch (VirtualRobotException& /*e*/)
        {
        }
    }

    if (!robot)
    {
        VR_WARNING << " ERROR while creating robot" << endl;
        return;
    }
}

void showRobotWindow::updateModelNodeControls()
{
    std::vector<ModelJointPtr> joints = (UI.cBoxJointSets->currentIndex() == 0) ? robot->getJoints() : robot->getJointSet(UI.cBoxJointSets->currentText().toStdString())->getJoints();
    std::vector<ModelLinkPtr> links = (UI.cBoxLinkSets->currentIndex() == 0) ? robot->getLinks() : robot->getLinkSet(UI.cBoxLinkSets->currentText().toStdString())->getLinks();

    // joints tab
    // joint table
    UI.tableJoints->clear();
    UI.tableJoints->setRowCount(joints.size());
    UI.tableJoints->setColumnCount(2);
    UI.tableJoints->setHorizontalHeaderLabels({"Name", "Joint Value"});
    for (int i = 0; i < joints.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(joints[i]->getName()));
        UI.tableJoints->setItem(i, 0, item);

        QSlider *slider = new JointValueSlider(joints[i], Qt::Horizontal);
        slider->setRange(0, 1000);
        float sliderValue = 1000 * ((joints[i]->getJointValue() - joints[i]->getJointLimitLow()) / (joints[i]->getJointLimitHigh() - joints[i]->getJointLimitLow()));
        slider->setValue(sliderValue);
        slider->setTracking(true);
        slider->setEnabled(joints[i]->getJointLimitHigh() - joints[i]->getJointLimitLow() != 0.0);
        UI.tableJoints->setCellWidget(i, 1, slider);

        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateJoints()));
    }

    // links tab
    // link list
    UI.listLinks->clear();
    for (std::size_t i = 0; i < links.size(); i++)
    {
        UI.listLinks->addItem(QString::fromStdString(links[i]->getName()));
    }
}

void showRobotWindow::updateJoints()
{
    std::map<std::string, float> jointValues;
    for (int i = 0; i < UI.tableJoints->rowCount(); i++)
    {
        std::string name = UI.tableJoints->item(i, 0)->text().toStdString();
        QSlider* slider = ((QSlider*) UI.tableJoints->cellWidget(i, 1));
        float ratio = (float)(slider->value() - slider->minimum()) / (slider->maximum() - slider->minimum());
        float value = robot->getJoint(name)->getJointLimitLow() + ratio * (robot->getJoint(name)->getJointLimitHigh() - robot->getJoint(name)->getJointLimitLow());
        jointValues[name] = value;
    }
    robot->setJointValues(jointValues);
}

void showRobotWindow::robotStructure()
{
    if (!robot)
    {
        return;
    }

    if (UI.checkBoxStructure->checkState() == Qt::Checked)
        robot->attachStructure(VirtualRobot::CoinVisualizationFactory::getName());
    else
        robot->detachStructure();

    rebuildVisualization();
}

void showRobotWindow::robotCoordSystems()
{
    if (!robot)
    {
        return;
    }

    if (UI.checkBoxRobotCoordSystems->checkState() == Qt::Checked)
        robot->attachFrames(VirtualRobot::CoinVisualizationFactory::getName());
    else
        robot->detachFrames();

    rebuildVisualization();
}

void showRobotWindow::closeHand()
{
    if (currentEEF)
    {
        currentEEF->closeActors();
    }
}

void showRobotWindow::openHand()
{
    if (currentEEF)
    {
        currentEEF->openActors();
    }
}

void showRobotWindow::selectEEF(int nr)
{
    cout << "Selecting EEF nr " << nr << endl;

     UI.comboBoxEndEffectorPS->clear();
     currentEEF.reset();

    if (nr < 0 || nr >= (int)eefs.size())
    {        
        return;
    }
    currentEEF = eefs[nr];

    std::vector<std::string> ps = currentEEF->getPreshapes();
    UI.comboBoxEndEffectorPS->addItem(QString("none"));
    for (unsigned int i = 0; i < ps.size(); i++)
    {
        UI.comboBoxEndEffectorPS->addItem(QString(ps[i].c_str()));
    }
}

void showRobotWindow::selectPreshape(int nr)
{
    cout << "Selecting EEF preshape nr " << nr << endl;

    if (!currentEEF || nr==0)
        return;

    nr--; // first entry is "none"

    std::vector<std::string> ps = currentEEF->getPreshapes();
    if (nr < 0 || nr >= (int)ps.size())
    {
        return;
    }

    VirtualRobot::RobotConfigPtr c = currentEEF->getPreshape(ps.at(nr));

    robot->setConfig(c);
}

void showRobotWindow::updateEEFBox()
{
    UI.comboBoxEndEffector->clear();

    for (unsigned int i = 0; i < eefs.size(); i++)
    {
        UI.comboBoxEndEffector->addItem(QString(eefs[i]->getName().c_str()));
    }
}
