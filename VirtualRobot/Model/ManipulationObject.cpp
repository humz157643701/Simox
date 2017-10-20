
#include "ManipulationObject.h"
#include "../VirtualRobotException.h"
#include "../Visualization/VisualizationNode.h"
#include "../Grasping/GraspSet.h"
#include "../CollisionDetection/CollisionChecker.h"
#include "../XML/BaseIO.h"

namespace VirtualRobot
{

    ManipulationObject::ManipulationObject(const std::string& name)
        : Obstacle(name)
    {
    }

    ManipulationObject::~ManipulationObject()
    {
    }

    void ManipulationObject::print(bool printDecoration)
    {
        if (printDecoration)
        {
            cout << "**** Manipulation Object ****" << endl;
        }

        Obstacle::print(false);

        for (size_t i = 0; i < graspSets.size(); i++)
        {
            cout << "* Grasp set " << i << ":" << endl;
            graspSets[i]->print();
        }

        if (printDecoration)
        {
            cout << endl;
        }
    }

    VirtualRobot::ManipulationObjectPtr ManipulationObject::create(const std::string& name, const VisualizationNodePtr& visualization, const CollisionModelPtr& collisionModel, const ModelLink::Physics& p, const CollisionCheckerPtr& colChecker)
    {
        ManipulationObjectPtr m(new ManipulationObject(name));
        ModelLinkPtr node(new ModelLink(m,
            name,
            Eigen::Matrix4f::Identity(),
            visualization,
            collisionModel,
            p,
            colChecker ? colChecker : CollisionChecker::getGlobalCollisionChecker()));

        m->registerModelNode(node);
        m->setRootNode(node);
        m->setGlobalPose(Eigen::Matrix4f::Identity());

        return m;
    }


    void ManipulationObject::addGraspSet(GraspSetPtr graspSet)
    {
        THROW_VR_EXCEPTION_IF(!graspSet, "NULL data");
        THROW_VR_EXCEPTION_IF(hasGraspSet(graspSet), "Grasp set already added");
        // don't be too strict
        //THROW_VR_EXCEPTION_IF(hasGraspSet(graspSet->getRobotType(), graspSet->getEndEffector()), "Only one GraspSet per EEF allowed.");
        this->graspSets.push_back(graspSet);
    }

    void ManipulationObject::includeGraspSet(GraspSetPtr toBeIncludedGraspSet)  //maybe delete
    {
        THROW_VR_EXCEPTION_IF(!toBeIncludedGraspSet,"NULL data");
        std::string robotType=toBeIncludedGraspSet->getRobotType();
        std::string eef=toBeIncludedGraspSet->getEndEffector();

        //include new Grasps
        //check if grasp is existing
        int index=-1;
        for(size_t i = 0 ; i < graspSets.size(); i++ )
        {
            if (graspSets.at(i)->getRobotType() == robotType && graspSets.at(i)->getEndEffector() == eef)
            {
                index = i;
            }
        }
        THROW_VR_EXCEPTION_IF(index==-1,"Index wrong defined");
        graspSets.at(index)->includeGraspSet(toBeIncludedGraspSet);
    }

    bool ManipulationObject::hasGraspSet(GraspSetPtr graspSet)
    {
        VR_ASSERT_MESSAGE(graspSet, "NULL data");

        for (size_t i = 0; i < graspSets.size(); i++)
            if (graspSets[i] == graspSet)
            {
                return true;
            }

        return false;
    }

    bool ManipulationObject::hasGraspSet(const std::string& robotType, const std::string& eef)
    {
        for (size_t i = 0; i < graspSets.size(); i++)
            if (graspSets[i]->getRobotType() == robotType && graspSets[i]->getEndEffector() == eef)
            {
                return true;
            }

        return false;
    }

    VirtualRobot::GraspSetPtr ManipulationObject::getGraspSet(EndEffectorPtr eef)
    {
        THROW_VR_EXCEPTION_IF(!eef, "NULL data");

        return getGraspSet(eef->getRobotType(), eef->getName());


    }

    VirtualRobot::GraspSetPtr ManipulationObject::getGraspSet(const std::string& robotType, const std::string& eefName)
    {
        for (size_t i = 0; i < graspSets.size(); i++)
            if (graspSets[i]->getRobotType() == robotType && graspSets[i]->getEndEffector() == eefName)
            {
                return graspSets[i];
            }

        return GraspSetPtr();
    }

    VirtualRobot::GraspSetPtr ManipulationObject::getGraspSet(const std::string& name)
    {
        for (size_t i = 0; i < graspSets.size(); i++)
            if (graspSets[i]->getName() == name)
            {
                return graspSets[i];
            }

        return GraspSetPtr();
    }

    std::vector<GraspSetPtr> ManipulationObject::getAllGraspSets()
    {
        return graspSets;
    }

    std::string ManipulationObject::toXML(const std::string& basePath, int tabs, bool storeLinkToFile)
    {
        std::stringstream ss;
        std::string t = "\t";
        std::string pre = "";

        for (int i = 0; i < tabs; i++)
        {
            pre += "\t";
        }

        ss << pre << "<ManipulationObject name='" << name << "'>\n";

        if (storeLinkToFile && !filename.empty())
        {
            std::string relFile = filename;

            if (!basePath.empty())
            {
                BaseIO::makeRelativePath(basePath, relFile);
            }

            ss << pre << t << "<File>" << relFile << "</File>\n";
            Eigen::Matrix4f gp = getGlobalPose();

            if (!gp.isIdentity())
            {
                ss << pre << t << "<GlobalPose>\n";
                ss << pre << t  << t  << "<Transform>\n";
                ss << BaseIO::getTransformXMLString(gp, tabs + 3);
                ss << pre << t  << t  << "</Transform>\n";
                ss << pre << t << "</GlobalPose>\n";
            }
        }
        else
        {
            // todo
            //ss << getSceneObjectXMLString(basePath, tabs + 1);

            for (size_t i = 0; i < graspSets.size(); i++)
            {
                ss << graspSets[i]->getXMLString(tabs + 1) << "\n";
            }
        }

        ss << pre << "</ManipulationObject>\n";

        return ss.str();
    }

<<<<<<< HEAD:VirtualRobot/Model/ManipulationObject.cpp
    ManipulationObjectPtr ManipulationObject::clone(const std::string &name, CollisionCheckerPtr colChecker, float scaling) const
    {
        ReadLockPtr r = getReadLock();
        ModelNodePtr startNode = getRootNode();
        THROW_VR_EXCEPTION_IF(!hasModelNode(startNode), " StartJoint is not part of this robot");
        THROW_VR_EXCEPTION_IF(scaling <= 0, " Scaling must be >0.");

        ManipulationObjectPtr result(new ManipulationObject(name));
        _clone(result, startNode, colChecker, true, true, scaling);
        result->filename = filename;
=======
    ManipulationObjectPtr ManipulationObject::clone(const std::string &name, CollisionCheckerPtr colChecker, bool deepVisuCopy) const
    {
        return ManipulationObjectPtr(_clone(name, colChecker, deepVisuCopy));
    }

    ManipulationObject* ManipulationObject::_clone(const std::string& name, CollisionCheckerPtr colChecker, bool deepVisuCopy) const
    {
        VisualizationNodePtr clonedVisualizationNode;

        if (visualizationModel)
        {
            clonedVisualizationNode = visualizationModel->clone(deepVisuCopy);
        }

        CollisionModelPtr clonedCollisionModel;

        if (collisionModel)
        {
            clonedCollisionModel = collisionModel->clone(colChecker, 1.0, deepVisuCopy);
        }

        ManipulationObject* result = new ManipulationObject(name, clonedVisualizationNode, clonedCollisionModel, physics, colChecker);

        if (!result)
        {
            VR_ERROR << "Cloning failed.." << endl;
            return result;
        }

        result->setGlobalPose(getGlobalPose());

>>>>>>> origin/master:VirtualRobot/ManipulationObject.cpp
        for (size_t i = 0; i < graspSets.size(); i++)
        {
            result->addGraspSet(graspSets[i]->clone());
        }
        return result;
    }

    VirtualRobot::ManipulationObjectPtr ManipulationObject::createFromMesh(TriMeshModelPtr mesh, const std::string &name, const std::string &visualizationType, CollisionCheckerPtr colChecker)
    {
        THROW_VR_EXCEPTION_IF(!mesh, "Null data");

        ManipulationObjectPtr result;
        VisualizationFactoryPtr visualizationFactory;

        if (visualizationType.empty())
        {
            visualizationFactory = VisualizationFactory::getGlobalVisualizationFactory();
        }
        else
        {
            visualizationFactory = VisualizationFactory::fromName(visualizationType, NULL);
        }

        if (!visualizationFactory)
        {
            VR_ERROR << "Could not create factory for visu type " << visualizationType << endl;
            return result;
        }


        Eigen::Matrix4f gp = Eigen::Matrix4f::Identity();
        VisualizationNodePtr visu = visualizationFactory->createTriMeshModelVisualization(mesh, gp);

        if (!visu)
        {
            VR_ERROR << "Could not create sphere visualization with visu type " << visualizationType << endl;
            return result;
        }

        //int id = idCounter;
        //idCounter++;
        std::string n;
        if (name.empty())
            n = "Mesh";

        CollisionModelPtr colModel(new CollisionModel(visu->clone(), name, colChecker));
        result = ManipulationObject::create(name, visu, colModel, ModelLink::Physics(), colChecker);

        return result;
    }


} //  namespace


