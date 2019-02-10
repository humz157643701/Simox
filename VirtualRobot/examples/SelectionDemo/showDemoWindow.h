﻿
#pragma once

#include <VirtualRobot/VirtualRobot.h>
#include <Gui/ViewerFactory.h>

#include <QtCore/QtGlobal>
#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include "ui_SelectionDemo.h"

class showDemoWindow : public QMainWindow
{
    Q_OBJECT
public:
    showDemoWindow();
    ~showDemoWindow();

public slots:
    /*! Closes the window and exits SoQt runloop. */
    void quit();

    /*!< Overriding the close event, so we know when the window was closed by the user. */
    virtual void closeEvent(QCloseEvent* event) override;

    void selectionModeChanged(QAbstractButton* button);
    void selectionButtonChecked_box_red(bool checked);
    void selectionButtonChecked_box_green(bool checked);
    void selectionButtonChecked_box_blue(bool checked);
    void selectionButtonChecked_cylinder_red(bool checked);
    void selectionButtonChecked_cylinder_green(bool checked);
    void selectionButtonChecked_cone_blue(bool checked);
    void groupShapes();
    void groupColors();
    void ungroup();
    void manipNone();
    void manipTranslate();
    void manipRotate();
    void manipTranslateRotate();

private:
    void setupUI();
    Ui::MainWindowSelectionDemo UI;

    SimoxGui::AbstractViewerPtr viewer;
    VirtualRobot::VisualizationPtr box_red;
    VirtualRobot::VisualizationPtr box_green;
    VirtualRobot::VisualizationPtr box_blue;
    VirtualRobot::VisualizationPtr cylinder_red;
    VirtualRobot::VisualizationPtr cylinder_green;
    VirtualRobot::VisualizationPtr cone_blue;

    VirtualRobot::VisualizationSetPtr boxes, cylinders, cones;
    VirtualRobot::VisualizationSetPtr red, green, blue;
};