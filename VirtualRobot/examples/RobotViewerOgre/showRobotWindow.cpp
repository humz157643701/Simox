
#include "showRobotWindow.h"
#include "VirtualRobot/EndEffector/EndEffector.h"
#include "VirtualRobot/Workspace/Reachability.h"
#include <VirtualRobot/RuntimeEnvironment.h>
#include <VirtualRobot/Import/RobotImporterFactory.h>
#include <VirtualRobot/Visualization/OgreVisualization/OgreVisualization.h>
#include <boost/algorithm/string/predicate.hpp>

#include <QFileDialog>
#include <Eigen/Geometry>

#include <time.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>

#include <VirtualRobot/Visualization/OgreVisualization/OgreVisualizationNode.h>

#include "VirtualRobot/MathTools.h"

using namespace std;
using namespace VirtualRobot;

float TIMER_MS = 30.0f;

showRobotWindow::showRobotWindow(std::string& sRobotFilename)
    : QMainWindow(NULL)
{
    VR_INFO << " start " << endl;

    useColModel = false;
    VirtualRobot::RuntimeEnvironment::getDataFileAbsolute(sRobotFilename);
    m_sRobotFilename = sRobotFilename;

    setupUI();

    loadRobot();

    //viewer->viewAll();
}

showRobotWindow::~showRobotWindow()
{
    robot.reset();
}

void showRobotWindow::setupUI()
{
    UI.setupUi(this);
    //centralWidget()->setLayout(UI.gridLayoutViewer);

    viewer = new SimoxGui::OgreViewer(UI.frameViewer);

    connect(UI.pushButtonReset, SIGNAL(clicked()), this, SLOT(resetSceneryAll()));
    connect(UI.pushButtonLoad, SIGNAL(clicked()), this, SLOT(selectRobot()));

    connect(UI.pushButtonClose, SIGNAL(clicked()), this, SLOT(closeHand()));
    connect(UI.ExportXML, SIGNAL(clicked()), this, SLOT(exportXML()));
    connect(UI.pushButtonOpen, SIGNAL(clicked()), this, SLOT(openHand()));
    connect(UI.comboBoxEndEffector, SIGNAL(activated(int)), this, SLOT(selectEEF(int)));

    connect(UI.checkBoxPhysicsCoM, SIGNAL(clicked()), this, SLOT(displayPhysics()));
    connect(UI.checkBoxPhysicsInertia, SIGNAL(clicked()), this, SLOT(displayPhysics()));

    connect(UI.checkBoxColModel, SIGNAL(clicked()), this, SLOT(rebuildVisualization()));
    connect(UI.checkBoxRobotSensors, SIGNAL(clicked()), this, SLOT(showSensors()));
    connect(UI.checkBoxStructure, SIGNAL(clicked()), this, SLOT(robotStructure()));
    UI.checkBoxFullModel->setChecked(true);
    connect(UI.checkBoxFullModel, SIGNAL(clicked()), this, SLOT(robotFullModel()));
    connect(UI.checkBoxRobotCoordSystems, SIGNAL(clicked()), this, SLOT(robotCoordSystems()));
    connect(UI.checkBoxShowCoordSystem, SIGNAL(clicked()), this, SLOT(showCoordSystem()));
    connect(UI.comboBoxRobotNodeSet, SIGNAL(activated(int)), this, SLOT(selectRNS(int)));
    connect(UI.comboBoxJoint, SIGNAL(activated(int)), this, SLOT(selectJoint(int)));
    connect(UI.horizontalSliderPos, SIGNAL(valueChanged(int)), this, SLOT(jointValueChanged(int)));

}

QString showRobotWindow::formatString(const char* s, float f)
{
    QString str1(s);

    if (f >= 0)
    {
        str1 += " ";
    }

    if (fabs(f) < 1000)
    {
        str1 += " ";
    }

    if (fabs(f) < 100)
    {
        str1 += " ";
    }

    if (fabs(f) < 10)
    {
        str1 += " ";
    }

    QString str1n;
    str1n.setNum(f, 'f', 3);
    str1 = str1 + str1n;
    return str1;
}


void showRobotWindow::resetSceneryAll()
{
    if (!robot)
    {
        return;
    }

    std::vector<float> jv(allRobotNodes.size(), 0.0f);
    robot->setJointValues(allRobotNodes, jv);

    selectJoint(UI.comboBoxJoint->currentIndex());
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

    if (currentRobotNodeSet)
    {
        trisRNSFull = currentRobotNodeSet->getNumFaces(false);
        trisRNSCol = currentRobotNodeSet->getNumFaces(true);
    }

    if (currentRobotNode)
    {
        trisJointFull = currentRobotNode->getNumFaces(false);
        trisJointCol = currentRobotNode->getNumFaces(true);
    }

    if (UI.checkBoxColModel->checkState() == Qt::Checked)
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

    bool showFullModel = UI.checkBoxFullModel->checkState() == Qt::Checked;

    robot->setupVisualization(showFullModel, true);

}

void showRobotWindow::rebuildVisualization()
{
    if (!robot)
    {
        return;
    }

    viewer->clearLayer("robotLayer");

    useColModel = UI.checkBoxColModel->checkState() == Qt::Checked;
    //bool sensors = UI.checkBoxRobotSensors->checkState() == Qt::Checked;
    VisualizationFactory::VisualizationType colModel = (UI.checkBoxColModel->isChecked()) ? VisualizationFactory::Collision : VisualizationFactory::Full;
   
    VisualizationFactoryPtr visualizationFactory = VisualizationFactory::first(NULL);

#if 1
    // Test the box creation and visualization

    auto boxA = visualizationFactory->createBox(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
    auto* realBoxA = dynamic_cast<OgreVisualizationNode*>(boxA.get());
    realBoxA->getOgreVisualization()->setPosition(0.0f, 0.0f, 0.0f);
    viewer->addVisualization("robotLayer", "boxA", boxA);

    auto boxB = visualizationFactory->createBox(1.0f, 1.0f, 1.0f, 0.0f, 0.7f, 0.0f);
    auto* realBoxB = dynamic_cast<OgreVisualizationNode*>(boxB.get());
    realBoxB->getOgreVisualization()->setPosition(3.0f, 0.0f, 0.0f);
    viewer->addVisualization("robotLayer", "boxB", boxB);

    auto boxC = visualizationFactory->createBox(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.7f);
    auto* realBoxC = dynamic_cast<OgreVisualizationNode*>(boxC.get());
    realBoxC->getOgreVisualization()->setPosition(-3.0f, 0.0f, 0.0f);
    viewer->addVisualization("robotLayer", "boxC", boxC);

    auto sphereA = visualizationFactory->createSphere(1.0f, 1.0f, 0.0f, 0.0f);
    auto* realSphereA = dynamic_cast<OgreVisualizationNode*>(sphereA.get());
    realSphereA->getOgreVisualization()->setPosition(0.0f, 0.0f, -3.0f);
    viewer->addVisualization("robotLayer", "sphereA", sphereA);

    auto sphereB = visualizationFactory->createSphere(1.0f, 0.0f, 0.7f, 0.0f);
    auto* realSphereB = dynamic_cast<OgreVisualizationNode*>(sphereB.get());
    realSphereB->getOgreVisualization()->setPosition(3.0f, 0.0f, -3.0f);
    viewer->addVisualization("robotLayer", "sphereB", sphereB);

    auto sphereC = visualizationFactory->createSphere(1.0f, 0.0f, 0.0f, 0.7f);
    auto* realSphereC = dynamic_cast<OgreVisualizationNode*>(sphereC.get());
    realSphereC->getOgreVisualization()->setPosition(-3.0f, 0.0f, -3.0f);
    viewer->addVisualization("robotLayer", "sphereC", sphereC);

    auto lineA = visualizationFactory->createLine(Eigen::Vector3f(4, -0.5f, 3), Eigen::Vector3f(-4, -0.5f, 3), 1.0f, 1, 0, 0);
    viewer->addVisualization("robotLayer", "lineA", lineA);

    auto lineB = visualizationFactory->createLine(Eigen::Vector3f(4, 0 , 3), Eigen::Vector3f(-4, 0 , 3), 1.0f, 0, 1, 0);
    viewer->addVisualization("robotLayer", "lineB", lineB);

    auto lineC = visualizationFactory->createLine(Eigen::Vector3f(4, 0.5f, 3), Eigen::Vector3f(-4, 0.5f, 3), 1.0f, 0, 0, 1);
    viewer->addVisualization("robotLayer", "lineC", lineC);

    viewer->setCameraTarget(sphereA);
#endif

    VisualizationPtr visu = visualizationFactory->getVisualization(robot, colModel);
    viewer->addVisualization("robotLayer", "robot", visu);

    selectJoint(UI.comboBoxJoint->currentIndex());

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

    std::vector<SensorPtr> sensors = robot->getSensors();

    for (size_t i = 0; i < sensors.size(); i++)
    {
        sensors[i]->setupVisualization(showSensors, showSensors);
        sensors[i]->showCoordinateSystem(showSensors);
    }

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
    robot->showPhysicsInformation(physicsCoMEnabled, physicsInertiaEnabled);

    // rebuild visualization
    rebuildVisualization();

}

void showRobotWindow::showRobot()
{
    //m_pGraspScenery->showRobot(m_pShowRobot->state() == QCheckBox::On);
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
    std::cout << "CShowRobotWindow: Closing" << std::endl;
    this->close();
    viewer->stop();
}

void showRobotWindow::updateJointBox()
{
    UI.comboBoxJoint->clear();

    for (unsigned int i = 0; i < currentRobotNodes.size(); i++)
    {
        UI.comboBoxJoint->addItem(QString(currentRobotNodes[i]->getName().c_str()));
    }
}

void showRobotWindow::updateRNSBox()
{
    UI.comboBoxRobotNodeSet->clear();
    UI.comboBoxRobotNodeSet->addItem(QString("<All>"));

    for (unsigned int i = 0; i < robotNodeSets.size(); i++)
    {
        UI.comboBoxRobotNodeSet->addItem(QString(robotNodeSets[i]->getName().c_str()));
    }
}

void showRobotWindow::selectRNS(int nr)
{
    currentRobotNodeSet.reset();
    cout << "Selecting RNS nr " << nr << endl;

    if (nr <= 0)
    {
        // all joints
        currentRobotNodes = allRobotNodes;
    }
    else
    {
        nr--;

        if (nr >= (int)robotNodeSets.size())
        {
            return;
        }

        currentRobotNodeSet = robotNodeSets[nr];
        currentRobotNodes = currentRobotNodeSet->getAllRobotNodes();
        /*cout << "HIGHLIGHTING rns " << currentRobotNodeSet->getName() << endl;
        if (visualization)
        {

            robot->highlight(visualization,false);
            currentRobotNodeSet->highlight(visualization,true);
        }*/

    }

    updateJointBox();
    selectJoint(0);
    displayTriangles();
}

void showRobotWindow::selectJoint(int nr)
{
    if (currentRobotNode)
    {
        currentRobotNode->showBoundingBox(false);
    }

    currentRobotNode.reset();
    cout << "Selecting Joint nr " << nr << endl;

    if (nr < 0 || nr >= (int)currentRobotNodes.size())
    {
        return;
    }

    currentRobotNode = currentRobotNodes[nr];
    currentRobotNode->showBoundingBox(true, true);
    currentRobotNode->print();
    float mi = currentRobotNode->getJointLimitLo();
    float ma = currentRobotNode->getJointLimitHi();
    QString qMin = QString::number(mi);
    QString qMax = QString::number(ma);
    UI.labelMinPos->setText(qMin);
    UI.labelMaxPos->setText(qMax);
    float j = currentRobotNode->getJointValue();
    UI.lcdNumberJointValue->display((double)j);

    if (fabs(ma - mi) > 0 && (currentRobotNode->isTranslationalJoint() || currentRobotNode->isRotationalJoint()))
    {
        UI.horizontalSliderPos->setEnabled(true);
        int pos = (int)((j - mi) / (ma - mi) * 1000.0f);
        UI.horizontalSliderPos->setValue(pos);
    }
    else
    {
        UI.horizontalSliderPos->setValue(500);
        UI.horizontalSliderPos->setEnabled(false);
    }

    if (currentRobotNodes[nr]->showCoordinateSystemState())
    {
        UI.checkBoxShowCoordSystem->setCheckState(Qt::Checked);
    }
    else
    {
        UI.checkBoxShowCoordSystem->setCheckState(Qt::Unchecked);
    }

    cout << "HIGHLIGHTING node " << currentRobotNodes[nr]->getName() << endl;

    if (visualization)
    {
        robot->highlight(visualization, false);
        currentRobotNode->highlight(visualization, true);
    }

    displayTriangles();
}

void showRobotWindow::jointValueChanged(int pos)
{
    int nr = UI.comboBoxJoint->currentIndex();

    if (nr < 0 || nr >= (int)currentRobotNodes.size())
    {
        return;
    }

    float fPos = currentRobotNodes[nr]->getJointLimitLo() + (float)pos / 1000.0f * (currentRobotNodes[nr]->getJointLimitHi() - currentRobotNodes[nr]->getJointLimitLo());
    robot->setJointValue(currentRobotNodes[nr], fPos);
    UI.lcdNumberJointValue->display((double)fPos);

#if 0
    RobotNodePtr rnl = robot->getRobotNode("LeftLeg_TCP");
    RobotNodePtr rnr = robot->getRobotNode("RightLeg_TCP");

    if (rnl && rnr)
    {
        cout << "LEFT:" << endl;
        MathTools::printMat(rnl->getGlobalPose());
        cout << "RIGHT:" << endl;
        MathTools::printMat(rnr->getGlobalPose());
    }

#endif
}

void showRobotWindow::showCoordSystem()
{
    float size = 0.75f;
    int nr = UI.comboBoxJoint->currentIndex();

    if (nr < 0 || nr >= (int)currentRobotNodes.size())
    {
        return;
    }

    currentRobotNodes[nr]->showCoordinateSystem(UI.checkBoxShowCoordSystem->checkState() == Qt::Checked, size);
    // rebuild visualization
    rebuildVisualization();
}



void showRobotWindow::selectRobot()
{
    string supportedExtensions = RobotImporterFactory::getAllExtensions();
    string supported = "Supported Formats, " + supportedExtensions + " (" + supportedExtensions + ")";
    string filter = supported + ";;" + RobotImporterFactory::getAllFileFilters();
    QString fi = QFileDialog::getOpenFileName(this, tr("Open Robot File"), QString(), tr(filter.c_str()));
    std::string s = m_sRobotFilename = std::string(fi.toLatin1());

    if (!s.empty())
    {
        m_sRobotFilename = s;
        loadRobot();
    }
}

void showRobotWindow::testPerformance(RobotPtr robot, RobotNodeSetPtr rns)
{
    int loops = 10000;
    Eigen::VectorXf limitMin(rns->getSize());
    Eigen::VectorXf limitMax(rns->getSize());
    for (size_t i = 0; i < rns->getSize(); i++)
    {
        limitMin[i] = rns->getNode(i)->getJointLimitLo();
        limitMax[i] = rns->getNode(i)->getJointLimitHi();
    }
    Eigen::VectorXf v(rns->getSize());
    //float minV = rn->getJointLimitLo();
    //float maxV = rn->getJointLimitHi();

    clock_t start = clock();
    robot->setupVisualization(true, false);
    robot->setUpdateVisualization(true);
    robot->setUpdateCollisionModel(true);
    robot->setThreadsafe(true);
    for (int i = 0; i < loops; i++)
    {
        for (size_t k = 0; k < rns->getSize(); k++)
        {
            float p = float(rand() % 1000) / 1000.0f;
            v[k] = limitMin[k] + p * (limitMax[k] - limitMin[k]);
        }
        rns->setJointValues(v);
    }
    clock_t end = clock();
    float timeMS = (float)(end - start) / (float)CLOCKS_PER_SEC * 1000.0f;
    VR_INFO << "Time (visu on, thread on): " << timeMS / (float)loops << endl;

    start = clock();
    robot->setupVisualization(false, false);
    robot->setUpdateVisualization(false);
    robot->setUpdateCollisionModel(false);
    robot->setThreadsafe(true);
    for (int i = 0; i < loops; i++)
    {
        /*float v = float(rand() % 1000) / 1000.0f;
        v = minV + v * (maxV - minV);
        rn->setJointValue(v);*/
        for (size_t k = 0; k < rns->getSize(); k++)
        {
            float p = float(rand() % 1000) / 1000.0f;
            v[k] = limitMin[k] + p * (limitMax[k] - limitMin[k]);
        }
        rns->setJointValues(v);
    }
    end = clock();
    timeMS = (float)(end - start) / (float)CLOCKS_PER_SEC * 1000.0f;
    VR_INFO << "Time (visu off, thread on): " << timeMS / (float)loops << endl;

    start = clock();
    robot->setupVisualization(true, false);
    robot->setUpdateVisualization(true);
    robot->setUpdateCollisionModel(true);
    robot->setThreadsafe(false);
    for (int i = 0; i < loops; i++)
    {
        for (size_t k = 0; k < rns->getSize(); k++)
        {
            float p = float(rand() % 1000) / 1000.0f;
            v[k] = limitMin[k] + p * (limitMax[k] - limitMin[k]);
        }
        rns->setJointValues(v);
    }
    end = clock();
    timeMS = (float)(end - start) / (float)CLOCKS_PER_SEC * 1000.0f;
    VR_INFO << "Time (visu on, thread off): " << timeMS / (float)loops << endl;


    start = clock();
    robot->setupVisualization(false, false);
    robot->setUpdateVisualization(false);
    robot->setUpdateCollisionModel(false);
    robot->setThreadsafe(false);
    for (int i = 0; i < loops; i++)
    {
        for (size_t k = 0; k < rns->getSize(); k++)
        {
            float p = float(rand() % 1000) / 1000.0f;
            v[k] = limitMin[k] + p * (limitMax[k] - limitMin[k]);
        }
        rns->setJointValues(v);
    }
    end = clock();
    timeMS = (float)(end - start) / (float)CLOCKS_PER_SEC * 1000.0f;
    VR_INFO << "Time (visu off, thread off): " << timeMS / (float)loops << endl;

}

void showRobotWindow::loadRobot()
{
    viewer->clearLayer("robotLayer");

    cout << "Loading Robot from " << m_sRobotFilename << endl;
    currentEEF.reset();
    currentRobotNode.reset();
    currentRobotNodes.clear();
    currentRobotNodeSet.reset();
    robot.reset();

    try
    {
        QFileInfo fileInfo(m_sRobotFilename.c_str());
        std::string suffix(fileInfo.suffix().toLatin1());
        RobotImporterFactoryPtr importer = RobotImporterFactory::fromFileExtension(suffix, NULL);

        if (!importer)
        {
            cout << " ERROR while grabbing importer" << endl;
            return;
        }

        robot = importer->loadFromFile(m_sRobotFilename, RobotIO::eFull);


    }
    catch (VirtualRobotException& e)
    {
        cout << " ERROR while creating robot" << endl;
        cout << e.what();
        return;
    }

    if (!robot)
    {
        cout << " ERROR while creating robot" << endl;
        return;
    }
    updatRobotInfo();
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
        RobotIO::saveXML(robot, fn, fnPath);
    }

}

void showRobotWindow::updatRobotInfo()
{
    if (!robot)
    {
        return;
    }

    UI.checkBoxColModel->setChecked(false);
    UI.checkBoxFullModel->setChecked(true);
    UI.checkBoxPhysicsCoM->setChecked(false);
    UI.checkBoxPhysicsInertia->setChecked(false);
    UI.checkBoxRobotCoordSystems->setChecked(false);
    UI.checkBoxShowCoordSystem->setChecked(false);
    UI.checkBoxStructure->setChecked(false);

    // get nodes
    robot->getRobotNodes(allRobotNodes);
    robot->getRobotNodeSets(robotNodeSets);
    robot->getEndEffectors(eefs);
    updateEEFBox();
    updateRNSBox();
    selectRNS(0);

    if (allRobotNodes.size() == 0)
    {
        selectJoint(-1);
    }
    else
    {
        selectJoint(0);
    }

    if (eefs.size() == 0)
    {
        selectEEF(-1);
    }
    else
    {
        selectEEF(0);
    }

    displayTriangles();

    // build visualization
    rebuildVisualization();
    robotStructure();
    displayPhysics();
    //viewer->viewAll();
}

void showRobotWindow::robotStructure()
{
    if (!robot)
    {
        return;
    }

    structureEnabled = UI.checkBoxStructure->checkState() == Qt::Checked;
    robot->showStructure(structureEnabled);
    // rebuild visualization
    rebuildVisualization();
}

void showRobotWindow::robotCoordSystems()
{
    if (!robot)
    {
        return;
    }

    bool robotAllCoordsEnabled = UI.checkBoxRobotCoordSystems->checkState() == Qt::Checked;
    robot->showCoordinateSystems(robotAllCoordsEnabled);
    // rebuild visualization
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

    if (nr < 0 || nr >= (int)eefs.size())
    {
        return;
    }

    currentEEF = eefs[nr];
}

void showRobotWindow::updateEEFBox()
{
    UI.comboBoxEndEffector->clear();

    for (unsigned int i = 0; i < eefs.size(); i++)
    {
        UI.comboBoxEndEffector->addItem(QString(eefs[i]->getName().c_str()));
    }
}