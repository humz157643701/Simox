#pragma once

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "inventor.h"


#include <VirtualRobot/VirtualRobot.h>
#include <VirtualRobot/Robot.h>
#include <VirtualRobot/VirtualRobotException.h>
#include <VirtualRobot/Visualization/CoinVisualization/CoinVisualization.h>
#include <VirtualRobot/RuntimeEnvironment.h>
#include <VirtualRobot/Nodes/RobotNodeRevoluteFactory.h>
#include <VirtualRobot/Nodes/RobotNodeFixedFactory.h>
#include <VirtualRobot/Nodes/RobotNodePrismaticFactory.h>
#include <VirtualRobot/Nodes/PositionSensor.h>
#include <VirtualRobot/Visualization/TriMeshModel.h>




namespace Collada
{



    struct ColladaSimoxRobotNode : InventorRobotNode
    {
        ColladaSimoxRobotNode(VirtualRobot::RobotPtr simoxRobot, float scaleFactor) ;

        ~ColladaSimoxRobotNode();
        void initialize() override;
        VirtualRobot::RobotPtr simoxRobot;
        VirtualRobot::RobotNodePtr simoxRobotNode;
        float scaleFactor;
    };

    class ColladaSimoxRobot : public InventorRobot
    {
    private:
        VirtualRobot::RobotPtr simoxRobot;
        float scaleFactor;
    public:
        ColladaSimoxRobot(float scaleFactor) ;
        ColladaRobotNodePtr robotNodeFactory() override
        {
            return ColladaRobotNodePtr(new ColladaSimoxRobotNode(simoxRobot, scaleFactor));
        }

        void initialize();
        VirtualRobot::RobotPtr getSimoxRobot()
        {
            return simoxRobot;
        }

        ~ColladaSimoxRobot();

    };


} //namespace


