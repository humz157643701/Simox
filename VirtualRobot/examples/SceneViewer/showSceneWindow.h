
#pragma once

#include "../../../VirtualRobot/Model/Model.h"
#include "../../../VirtualRobot/VirtualRobotException.h"
#include "../../../VirtualRobot/Model/Nodes/ModelNode.h"
#include "../../../VirtualRobot/XML/SceneIO.h"
#include "../../../VirtualRobot/Visualization/VisualizationFactory.h"
#include "../../../VirtualRobot/Model/Obstacle.h"

#include "../../../Gui/AbstractViewer.h"
#include "../../../Gui/ViewerFactory.h"

#include <QtCore/QtGlobal>
#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include <string.h>
#include <vector>

#include "ui_SceneViewer.h"

class showSceneWindow : public QMainWindow
{
    Q_OBJECT
public:
    showSceneWindow(std::string& sSceneFile);
    ~showSceneWindow();

public slots:
    /*! Closes the window and exits SoQt runloop. */
    void quit();

    /*!< Overriding the close event, so we know when the window was closed by the user. */
    virtual void closeEvent(QCloseEvent* event) override;

    void resetSceneryAll();
    void loadScene();
    void selectScene();

    void selectRobot(int nr);
    void selectObject(int nr);
    void selectGrasp(int nr);
    void selectEEF(int nr);
    void selectRobotConfig(int nr);
    void selectTrajectory(int nr);
    void sliderMoved(int pos);

    void closeHand();
    void openHand();
    void colModel();
    void showRoot();

protected:

    void updateGui();
    void updateGrasps();
    void updateGraspVisu();
    void setupUI();
    void buildVisu();

    Ui::MainWindowShowScene UI;
    SimoxGui::AbstractViewerPtr viewer;

    VirtualRobot::GraspPtr currentGrasp;
    VirtualRobot::GraspSetPtr currentGraspSet;
    VirtualRobot::ModelPtr currentObject;
    VirtualRobot::RobotPtr currentRobot;
    VirtualRobot::TrajectoryPtr currentTrajectory;
    VirtualRobot::EndEffectorPtr currentEEF;

    VirtualRobot::ScenePtr scene;
    std::string sceneFile;

    VirtualRobot::VisualizationSetPtr visualization;
};
