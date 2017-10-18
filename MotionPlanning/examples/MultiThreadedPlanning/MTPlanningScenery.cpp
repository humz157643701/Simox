#include "MTPlanningScenery.h"

#include "VirtualRobot/XML/ModelIO.h"
#include "VirtualRobot/Visualization/Visualization.h"
#include "VirtualRobot/CollisionDetection/CollisionChecker.h"
#include "VirtualRobot/Tools/MathTools.h"
#include "VirtualRobot/Model/Model.h"
#include "VirtualRobot/EndEffector/EndEffector.h"
#include "VirtualRobot/Model/Nodes/ModelNode.h"
#include "VirtualRobot/Model/ModelNodeSet.h"
#include "VirtualRobot/Tools/RuntimeEnvironment.h"
#include "VirtualRobot/Model/Obstacle.h"
#include "MotionPlanning/Planner/Rrt.h"
#include "MotionPlanning/PostProcessing/ShortcutProcessor.h"
#include "VirtualRobot/CollisionDetection/CDManager.h"
#include "VirtualRobot/Import/SimoxXMLFactory.h"
#include <sstream>

using namespace std;
using namespace VirtualRobot;
using namespace MotionPlanning;

#define SHORTEN_LOOP 600

#ifdef Simox_USE_COIN_VISUALIZATION
    #include "../../../Gui/Coin/CoinViewerFactory.h"
    #include <MotionPlanning/Visualization/CoinVisualization/CoinRrtWorkspaceVisualization.h>

    // need this to ensure that static Factory methods are called across library boundaries (otherwise coin Gui lib is not loaded since it is not referenced by us)
    SimoxGui::CoinViewerFactory f2;
#endif


MTPlanningScenery::MTPlanningScenery(const std::string &robotFile, SimoxGui::ViewerInterfacePtr viewer)
    : viewer(viewer)
{
    robotModelVisuColModel = true;

    addBBCube();

    colModel = "colModel";
    kinChainName = "All";
    TCPName = "Visu";

    robotFilename = robotFile;
    VirtualRobot::RuntimeEnvironment::getDataFileAbsolute(robotFilename);

    plannersStarted = false;
    optimizeStarted = false;

    buildScene();
}

MTPlanningScenery::~MTPlanningScenery()
{
    startPositions.clear();
    goalPositions.clear();
}

void MTPlanningScenery::reset()
{
    for (int i = 0; i < (int)robots.size(); i++)
    {
        robots[i].reset();
    }

    robots.clear();

    if (plannersStarted)
    {
        stopPlanning();
    }

    if (optimizeStarted)
    {
        stopOptimizing();
    }

    for (int i = 0; i < (int)planners.size(); i++)
    {
        planners[i].reset();
    }

    planners.clear();

    for (int i = 0; i < (int)CSpaces.size(); i++)
    {
        CSpaces[i].reset();
    }

    CSpaces.clear();

    for (int i = 0; i < (int)planningThreads.size(); i++)
    {
        planningThreads[i]->stop();
        planningThreads[i].reset();
    }

    planningThreads.clear();

    for (int i = 0; i < (int)optimizeThreads.size(); i++)
    {
        optimizeThreads[i].reset();
    }

    optimizeThreads.clear();

    solutions.clear();
    optiSolutions.clear();

    viewer->clearLayer("visualizations");
    viewer->clearLayer("startgoal");
}

void MTPlanningScenery::buildScene()
{
    viewer->clearLayer("obstacles");

    float fCubeSize = 50.0f;
    float fPlayfieldSize = 1000.0f - fCubeSize;
    std::vector<ModelPtr> environmentModels;

    int ob = 2000;
    cout << "Randomly placing " << ob << " obstacles..." << endl;

    for (int i = 0; i < ob; i++)
    {
        VirtualRobot::ObstaclePtr o = VirtualRobot::Obstacle::createBox(fCubeSize, fCubeSize, fCubeSize);

        Eigen::Vector3f p;
        p(0) = (float)(rand() % (2 * (int)fPlayfieldSize) - (int)fPlayfieldSize);
        p(1) = (float)(rand() % (2 * (int)fPlayfieldSize) - (int)fPlayfieldSize);
        p(2) = (float)(rand() % (2 * (int)fPlayfieldSize) - (int)fPlayfieldSize);
        Eigen::Matrix4f m;
        m.setIdentity();
        m.block(0, 3, 3, 1) = p;
        o->setGlobalPose(m);
        environmentModels.push_back(o);
        VisualizationPtr v = VisualizationFactory::getGlobalVisualizationFactory()->getVisualization(o, ModelLink::Full);
        std::stringstream on;
        on << "Obstacle-" << i;
        viewer->addVisualization("obstacles", on.str(), v);
    }

    environment.reset(new ModelSet("ObstacleModels", environmentModels));
    environmentUnited = environment->createStaticObstacle("Obstacles");

    viewer->viewAll();
}

void MTPlanningScenery::getRandomPos(float& x, float& y, float& z)
{
    float fPlayfieldSize = 1000.0f;

    if (rand() % 2 == 0)
    {
        x = -fPlayfieldSize;
    }
    else
    {
        x = fPlayfieldSize;
    }

    y = (float)(rand() % (2 * (int)fPlayfieldSize) - (int)fPlayfieldSize);
    z = (float)(rand() % (2 * (int)fPlayfieldSize) - (int)fPlayfieldSize);
    int i = rand() % 3;

    if (i == 0)
    {
        float tt = x;
        x = y;
        y = tt;
    }
    else if (i == 1)
    {
        float tt = x;
        x = z;
        z = tt;
    }
}

void MTPlanningScenery::buildPlanningThread(bool bMultiCollisionCheckers, int id)
{
    if (!environmentUnited)
    {
        cout << "Build Environment first!..." << endl;
        return;
    }

    if (plannersStarted)
    {
        return;
    }

    cout << " Build planning thread ";

    if (bMultiCollisionCheckers)
    {
        cout << "with own instance of collision checker" << endl;
    }
    else
    {
        cout << "with collision checker singleton" << endl;
    }

    this->loadRobotMTPlanning(bMultiCollisionCheckers);

    if (this->robots.empty())
    {
        cout << "Could not load a robot!..." << endl;
        return;
    }

    RobotPtr pRobot = robots[robots.size() - 1];
    JointSetPtr kinChain = pRobot->getJointSet(kinChainName);

    CDManagerPtr pCcm(new VirtualRobot::CDManager(pRobot->getCollisionChecker()));
    cout << "Set CSpace for " << robots.size() << ".th robot." << endl;
    pCcm->addCollisionModel(pRobot->getLinkSet(colModel));
    ObstaclePtr pEnv = environmentUnited;

    if (bMultiCollisionCheckers)
    {
        //clone environment
        pEnv = environmentUnited->clone("Cloned Environment", pRobot->getCollisionChecker());
    }

    pCcm->addCollisionModel(pEnv);
    CSpaceSampledPtr pCSpace(new CSpaceSampled(pRobot, pCcm, kinChain));

    if (!bMultiCollisionCheckers)
    {
        pCSpace->exclusiveRobotAccess(true); // only needed, when one collision checker is shared between the threads
    }

    pCSpace->setSamplingSizeDCD(1.0f);
    pCSpace->setSamplingSize(20.0f);
    BiRrtPtr pPlanner(new BiRrt(pCSpace));

    //setup random start and goal
    float x, y, z;
    Eigen::VectorXf start(3);
    Eigen::VectorXf goal(3);
    pRobot->setUpdateVisualization(false);

    do
    {
        getRandomPos(x, y, z);

        start[0] = x;
        start[1] = y;
        start[2] = z;
        cout << "START: " << x << "," << y << "," << z << endl;
        kinChain->setJointValues(start);
    }
    while (pCcm->isInCollision());

    startPositions.push_back(start);

    do
    {
        getRandomPos(x, y, z);
        goal[0] = x;
        goal[1] = y;
        goal[2] = z;
        cout << "GOAL: " << x << "," << y << "," << z << endl;
        kinChain->setJointValues(goal);
    }
    while (pCcm->isInCollision());


    goalPositions.push_back(goal);

    pRobot->setUpdateVisualization(true);

    pPlanner->setStart(start);
    pPlanner->setGoal(goal);
    PlanningThreadPtr pThread(new PlanningThread(pPlanner));
    planners.push_back(pPlanner);
    CSpaces.push_back(pCSpace);
    planningThreads.push_back(pThread);
    solutions.push_back(CSpacePathPtr());
    optiSolutions.push_back(CSpacePathPtr());
    optimizeThreads.push_back(PathProcessingThreadPtr());
/*
    SoSphere* s = new SoSphere();
    s->radius = 30.0f;
    SoMaterial* mat = new SoMaterial();
    mat->ambientColor.setValue(1.0, 0, 0);
    mat->diffuseColor.setValue(1.0, 0, 0);
    SoMaterial* mat2 = new SoMaterial();
    mat2->ambientColor.setValue(0, 0, 1.0);
    mat2->diffuseColor.setValue(0, 0, 1.0);*/


    FramePtr rn = pRobot->getFrame(TCPName);

    if (!rn)
    {
        return;
    }

    kinChain->setJointValues(start);
    Eigen::Matrix4f gp = rn->getGlobalPose();
    VisualizationNodePtr v = VisualizationFactory::getGlobalVisualizationFactory()->createSphere(30.0f, 1.0f, 0, 0);
    VisualizationFactory::getGlobalVisualizationFactory()->applyDisplacement(v, gp);
    std::stringstream nameStart;
    nameStart << "start-point-" << id;
    viewer->addVisualization("startgoal", nameStart.str(), v);

    kinChain->setJointValues(goal);
    Eigen::Matrix4f gp2 = rn->getGlobalPose();
    VisualizationNodePtr v2 = VisualizationFactory::getGlobalVisualizationFactory()->createSphere(30.0f, 0, 0, 1.0f);
    VisualizationFactory::getGlobalVisualizationFactory()->applyDisplacement(v2, gp2);
    std::stringstream nameGoal;
    nameGoal << "goal-point-" << id;
    viewer->addVisualization("startgoal", nameGoal.str(), v2);
    //SoMatrixTransform* mt2 = CoinVisualizationFactory::getMatrixTransform(gp); //no transformation -> our scene is already in MM units

    std::stringstream nameStartText;
    nameStartText << "start-" << id;
    VisualizationNodePtr v1t = VisualizationFactory::getGlobalVisualizationFactory()->createText(nameStartText.str(), true, 7.0f, VisualizationFactory::Color::Black(), 10.0f, 0, 0);
    VisualizationFactory::getGlobalVisualizationFactory()->applyDisplacement(v1t, gp);
    viewer->addVisualization("startgoal", nameStartText.str(), v1t);

    std::stringstream nameGoalText;
    nameGoalText << "goal-" << id;
    VisualizationNodePtr v2t = VisualizationFactory::getGlobalVisualizationFactory()->createText(nameGoalText.str(), true, 7.0f, VisualizationFactory::Color::Black(), 10.0f, 0, 0);
    VisualizationFactory::getGlobalVisualizationFactory()->applyDisplacement(v2t, gp2);
    viewer->addVisualization("startgoal", nameGoalText.str(), v2t);
}


PathProcessingThreadPtr MTPlanningScenery::buildOptimizeThread(CSpaceSampledPtr cspace, CSpacePathPtr path)
{

    ShortcutProcessorPtr o(new ShortcutProcessor(path, cspace));
    PathProcessingThreadPtr optiThread(new PathProcessingThread(o));
    return optiThread;
}


void MTPlanningScenery::stopPlanning()
{
    cout << "Stopping " << planningThreads.size() << " planning threads..." << endl;

    for (unsigned int i = 0; i < planningThreads.size(); i++)
    {
        planningThreads[i]->stop();
    }

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        robots[i]->setUpdateVisualization(true);
    }

    cout << "... done" << endl;
    plannersStarted = false;
}


void MTPlanningScenery::stopOptimizing()
{
    if (!optimizeStarted)
    {
        cout << "Start the optimizing first!..." << endl;
        return;
    }

    cout << "Stopping " << optimizeThreads.size() << " optimizing threads..." << endl;

    for (int i = 0; i < (int)optimizeThreads.size(); i++)
    {
        optimizeThreads[i]->stop();
    }

    for (int i = 0; i < (int)robots.size(); i++)
    {
        robots[i]->setUpdateVisualization(true);
    }

    cout << "...done" << endl;
    optimizeStarted = false;
}


void MTPlanningScenery::startPlanning()
{
    if (plannersStarted)
    {
        cout << "already started!..." << endl;
        return;
    }

    cout << "Starting " << planningThreads.size() << " planning threads..." << endl;

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        robots[i]->setUpdateVisualization(false);
    }

    for (unsigned int i = 0; i < planningThreads.size(); i++)
    {
        planningThreads[i]->start();
    }

    cout << "... done" << endl;

    plannersStarted = true;
}


void MTPlanningScenery::startOptimizing()
{
    if (!plannersStarted)
    {
        cout << "Plan the solutions first!..." << endl;
        return;
    }

    if (CSpaces.empty() || solutions.empty())
    {
        cout << "Build planning threads first!..." << endl;
        return;
    }

    if (plannersStarted)
    {
        for (int i = 0; i < (int)planningThreads.size(); i++)
        {
            if (planningThreads[i]->isRunning())
            {
                cout << "Planning is not finish!..." << endl;
                return;
            }
        }
    }

    if (optimizeStarted)
    {
        cout << "Path processors already started..." << endl;
        return;
    }

    for (int i = 0; i < (int)solutions.size(); i++)
    {
        if (solutions[i])
        {
            optimizeThreads[i] = buildOptimizeThread(CSpaces[i], solutions[i]);
            optiSolutions[i].reset();
        }
    }

    int j = 0;

    for (unsigned int i = 0; i < robots.size(); i++)
    {
        robots[i]->setUpdateVisualization(false);
    }

    for (unsigned int i = 0; i < optimizeThreads.size(); i++)
    {
        if (optimizeThreads[i])
        {
            optimizeThreads[i]->start(SHORTEN_LOOP);
            j++;
        }
    }

    cout << "... done" << endl;
    cout << "Starting " << j << " path processing threads..." << endl;
    optimizeStarted = true;
}


void MTPlanningScenery::loadRobotMTPlanning(bool bMultiCollisionCheckers)
{
    CollisionCheckerPtr pColChecker = CollisionChecker::getGlobalCollisionChecker();

    if (bMultiCollisionCheckers)
    {
        pColChecker.reset(new CollisionChecker());
    }

    RobotPtr pRobot;

    if ((int)robots.size() == 0)
    {
        // xml parsing
        pRobot = SimoxXMLFactory::loadRobotSimoxXML(robotFilename);

        if (!pRobot)
        {
            cout << "Error parsing file " << robotFilename << ". Aborting" << endl;
            return;
        }

        RobotNodeSetPtr kinChain = pRobot->getModelNodeSet(kinChainName);

        if (!kinChain)
        {
            cout << "No rns " << kinChainName << ". Aborting" << endl;
            return;
        }

        // get robot
        cout << "Successfully read " << robotFilename << std::endl;
    }
    else
    {
        pRobot = robots[0];
    }

    if (!pRobot)
    {
        cout << "error while parsing xml file: no pRobot.." << std::endl;
        return;
    }

    std::string sNewName;
    std::ostringstream oss;
    oss << pRobot->getName() << "_" << robots.size();
    sNewName = oss.str();
    pRobot = pRobot->clone(sNewName, pColChecker);
    robots.push_back(pRobot);

    if ((int)robots.size() == 1)
    {
        VisualizationPtr v = VisualizationFactory::getGlobalVisualizationFactory()->getVisualization(robots[0], robotModelVisuColModel ? ModelLink::Full : ModelLink::Collision);
        viewer->addVisualization("robot", "robot", v);
    }

    int trFull = pRobot->getNumFaces(false);
    int trCol = pRobot->getNumFaces(true);

    cout << "Loaded/Cloned robot with " << trFull << "/" << trCol << " number of triangles." << endl;
    //reset();
    cout << "Loaded/Cloned " << (int)robots.size() << " robots..." << endl;
}


// constructs a bounding box cube for the rrt and adds it as child to result
void MTPlanningScenery::addBBCube()
{
    float lineSize = 2.0;
    float x1 = -1000.0f;
    float y1 = -1000.0f;
    float z1 = -1000.0f;
    float x2 = 1000.0f;
    float y2 = 1000.0f;
    float z2 = 1000.0f;

    Eigen::Vector3f p1(x1,y1,z1);
    Eigen::Vector3f p2(x2,y1,z1);
    Eigen::Vector3f p3(x2,y2,z1);
    Eigen::Vector3f p4(x1,y2,z1);

    Eigen::Vector3f p1b(x1,y1,z2);
    Eigen::Vector3f p2b(x2,y1,z2);
    Eigen::Vector3f p3b(x2,y2,z2);
    Eigen::Vector3f p4b(x1,y2,z2);

    VisualizationNodePtr v1 = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p1, p2, lineSize, 0, 0, 0);
    VisualizationNodePtr v2 = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p2, p3, lineSize, 0, 0, 0);
    VisualizationNodePtr v3 = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p3, p4, lineSize, 0, 0, 0);
    VisualizationNodePtr v4 = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p4, p1, lineSize, 0, 0, 0);
    viewer->addVisualization("bbox", "l1", v1);
    viewer->addVisualization("bbox", "l2", v2);
    viewer->addVisualization("bbox", "l3", v3);
    viewer->addVisualization("bbox", "l4", v4);

    VisualizationNodePtr v1b = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p1b, p2b, lineSize, 0, 0, 0);
    VisualizationNodePtr v2b = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p2b, p3b, lineSize, 0, 0, 0);
    VisualizationNodePtr v3b = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p3b, p4b, lineSize, 0, 0, 0);
    VisualizationNodePtr v4b = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p4b, p1b, lineSize, 0, 0, 0);
    viewer->addVisualization("bbox", "l1b", v1b);
    viewer->addVisualization("bbox", "l2b", v2b);
    viewer->addVisualization("bbox", "l3b", v3b);
    viewer->addVisualization("bbox", "l4b", v4b);

    VisualizationNodePtr v1c = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p1, p2, lineSize, 0, 0, 0);
    VisualizationNodePtr v2c = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p2, p2b, lineSize, 0, 0, 0);
    VisualizationNodePtr v3c = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p2b, p1b, lineSize, 0, 0, 0);
    VisualizationNodePtr v4c = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p1b, p1, lineSize, 0, 0, 0);
    viewer->addVisualization("bbox", "l1c", v1c);
    viewer->addVisualization("bbox", "l2c", v2c);
    viewer->addVisualization("bbox", "l3c", v3c);
    viewer->addVisualization("bbox", "l4c", v4c);

    VisualizationNodePtr v1d = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p4, p3, lineSize, 0, 0, 0);
    VisualizationNodePtr v2d = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p3, p3b, lineSize, 0, 0, 0);
    VisualizationNodePtr v3d = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p3b, p4b, lineSize, 0, 0, 0);
    VisualizationNodePtr v4d = VisualizationFactory::getGlobalVisualizationFactory()->createLine(p4b, p4, lineSize, 0, 0, 0);
    viewer->addVisualization("bbox", "l1d", v1d);
    viewer->addVisualization("bbox", "l2d", v2d);
    viewer->addVisualization("bbox", "l3d", v3d);
    viewer->addVisualization("bbox", "l4d", v4d);
}


void MTPlanningScenery::setRobotModelShape(bool collisionModel)
{
    robotModelVisuColModel = collisionModel;
}


void MTPlanningScenery::checkPlanningThreads()
{
    if (!plannersStarted)
    {
        return;
    }

    for (unsigned int i = 0; i < planningThreads.size(); i++)
    {
        if (!planningThreads[i]->isRunning())
        {
            if (!solutions[i])
            {
                CSpacePathPtr sol = planners[i]->getSolution();

                if (sol)
                {
                    solutions[i] = sol->clone();
                    VR_INFO << "fetching solution " << i << endl;

                    MotionPlanning::RrtWorkspaceVisualizationPtr w;
                    #ifdef Simox_USE_COIN_VISUALIZATION
                        w.reset(new MotionPlanning::CoinRrtWorkspaceVisualization(robots[i], CSpaces[i], TCPName));
                    #else
                        VR_ERROR << "NO VISUALIZATION IMPLEMENTATION SPECIFIED..." << endl;
                    #endif

                    w->addCSpacePath(solutions[i]);
                    w->addTree(planners[i]->getTree());
                    VisualizationPtr wv = w->getVisualization();
                    if (wv)
                    {
                        std::stringstream n;
                        n << "solution-orig-" << i;
                        viewer->addVisualization("solution",n.str(), wv);
                    }
                }
                else
                {
                    cout << "no solution in thread " << i << endl;
                }
            }
        }
    }
}


void MTPlanningScenery::checkOptimizeThreads()
{
    if (!optimizeStarted)
    {
        return;
    }

    for (int i = 0; i < (int)optimizeThreads.size(); i++)
    {
        if (!optimizeThreads[i]->isRunning())
        {
            CSpacePathPtr pOptiSol = optimizeThreads[i]->getProcessedPath();

            if (pOptiSol)
            {
                if (!optiSolutions[i])
                {
                    VR_INFO << "fetching optimized solution " << i << endl;
                    MotionPlanning::RrtWorkspaceVisualizationPtr w;
                    #ifdef Simox_USE_COIN_VISUALIZATION
                        w.reset(new MotionPlanning::CoinRrtWorkspaceVisualization(robots[i], CSpaces[i], TCPName));
                    #else
                        VR_ERROR << "NO VISUALIZATION IMPLEMENTATION SPECIFIED..." << endl;
                    #endif

                    optiSolutions[i] = pOptiSol->clone();
                    w->addCSpacePath(optiSolutions[i], RrtWorkspaceVisualization::ColorSet::eGreen);
                    VisualizationPtr wv = w->getVisualization();
                    if (wv)
                    {
                        std::stringstream n;
                        n << "solution-optimized-" << i;
                        viewer->addVisualization("solution",n.str(), wv);
                    }
                }
            }
            else
            {
                cout << "No optimized solution in thread " << i << endl;
                cout << "show the original solution" << endl;
            }
        }
    }
}


void MTPlanningScenery::getThreadCount(int& nWorking, int& nIdle)
{
    nWorking = 0;
    nIdle = 0;

    for (unsigned int i = 0; i < planningThreads.size(); i++)
    {
        PlanningThreadPtr pThread = planningThreads[i];

        if (pThread->isRunning())
        {
            nWorking++;
        }
        else
        {
            nIdle++;
        }
    }
}


void MTPlanningScenery::getOptimizeThreadCount(int& nWorking, int& nIdle)
{
    nWorking = 0;
    nIdle = 0;

    for (unsigned int i = 0; i < optimizeThreads.size(); i++)
    {
        PathProcessingThreadPtr pOptiThread = optimizeThreads[i];

        if (pOptiThread && pOptiThread->isRunning())
        {
            nWorking++;
        }
        else
        {
            nIdle++;
        }
    }
}

int MTPlanningScenery::getThreads()
{
    return (int)(planningThreads.size());
}
