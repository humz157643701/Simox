#include "SimpleCubeWindow.h"
#include "../../VirtualRobotException.h"

#include "../../Visualization/VisualizationFactory.h"

SimpleCubeWindow::SimpleCubeWindow()
    : QMainWindow(NULL)
{
    setupUI();
}


SimpleCubeWindow::~SimpleCubeWindow()
{

}


void SimpleCubeWindow::setupUI()
{
    UI.setupUi(this);

    SimoxGui::ViewerFactoryPtr viewerFactory = SimoxGui::ViewerFactory::getInstance();
    THROW_VR_EXCEPTION_IF(!viewerFactory,"No viewer factory?!");
    viewer = viewerFactory->createViewer(UI.frameViewer);

    VirtualRobot::VisualizationPtr visu = VirtualRobot::VisualizationFactory::getInstance()->createBox(1000.0f, 1000.0f, 1000.0f);
    viewer->addVisualization("test", visu);
    viewer->viewAll();
}

int SimpleCubeWindow::main()
{

}

