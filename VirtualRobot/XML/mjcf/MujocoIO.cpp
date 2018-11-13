#include "MujocoIO.h"
#include "utils.h"
#include "xml_visitors.h"

#include <VirtualRobot/RobotNodeSet.h>
#include <VirtualRobot/XML/RobotIO.h>
#include <VirtualRobot/VirtualRobotException.h>
#include <VirtualRobot/Visualization/TriMeshModel.h>


using namespace VirtualRobot;
using namespace mjcf;
namespace tx = tinyxml2; 
namespace fs = boost::filesystem;



MujocoIO::MujocoIO() :
    masslessBodySanitizer(document, robot)
{
}

void MujocoIO::saveMJCF(RobotPtr robot, const std::string& filename, const std::string& basePath, const std::string& meshDir)
{
    THROW_VR_EXCEPTION_IF(!robot, "Given RobotPtr robot is null.");
    THROW_VR_EXCEPTION_IF(filename.empty(), "Given filename is empty.");
    
    this->robot = robot;
    this->outputDirectory = basePath;
    this->outputFileName = filename;
    this->outputMeshRelDirectory = meshDir;
    
    document.reset(new mjcf::Document());
    document->setModelName(robot->getName());
    
    document->compiler()->SetAttribute("angle", "radian");
    document->compiler()->SetAttribute("balanceinertia", "true");
    
    Element* defaultsClass = document->addDefaultsClass(robot->getName());
    std::stringstream ss;
    ss << "Add default values for " << robot->getName() << " here.";
    defaultsClass->InsertFirstChild(document->NewComment(ss.str().c_str()));
    
    document->setNewElementClass(robot->getName(), true);
    
    
    makeEnvironment();
    
    std::cout << "Creating bodies structure ..." << std::endl;
    addNodeBodies();
    
    std::cout << "Adding meshes and geoms ..." << std::endl;
    addNodeBodyMeshes();
    
    std::cout << "===========================" << std::endl
              << "Current model: "             << std::endl
              << "--------------"              << std::endl;
    document->Print();
    std::cout << "===========================" << std::endl;
    
    std::cout << "Merging massless bodies ..." << std::endl;
    masslessBodySanitizer.sanitize();
    
    std::cout << "Adding contact excludes ..." << std::endl;
    addContactExcludes();

    std::cout << "Adding actuators ..." << std::endl;
    addActuators();
    
    std::cout << "Scaling lengths by " << lengthScaling << " ..." << std::endl;
    scaleLengths();
    
    std::cout << "Done.";
    
    std::cout << std::endl;
    std::cout << "===========================" << std::endl
              << "Output file: "             << std::endl
              << "------------"              << std::endl;
    document->Print();
    std::cout << "===========================" << std::endl;
    
    
    assert(!outputFileName.empty());
    std::cout << "Writing to " << (outputDirectory / outputFileName) << std::endl;
    document->SaveFile((outputDirectory / outputFileName).c_str());
}


void MujocoIO::makeEnvironment()
{
    document->addSkyboxTexture(Eigen::Vector3f(.8f, .9f, .95f), 
                               Eigen::Vector3f(.4f, .6f, .8f));
}


void MujocoIO::addNodeBodies()
{
    nodeBodies.clear();
    
    RobotNodePtr rootNode = robot->getRootNode();
    assert(rootNode);
    
    // add root
    Element* robotRootBody = document->addRobotRootBodyElement(robot->getName());
    
    Element* root = document->addBodyElement(robotRootBody, rootNode);
    nodeBodies[rootNode->getName()] = root;
    assert(root);
    
    for (RobotNodePtr node : robot->getRobotNodes())
    {
        addNodeBody(node);
    }
}

void MujocoIO::addNodeBodyMeshes()
{
    bool meshlabserverAviable = system("which meshlabserver > /dev/null 2>&1") == 0;
    bool notAvailableReported = false;
    
    for (RobotNodePtr node : robot->getRobotNodes())
    {
        VisualizationNodePtr visualization = node->getVisualization(SceneObject::VisualizationType::Full);
        
        if (!visualization)
        {
            continue;
        }
        
        std::cout << "Node " << node->getName() << ":\t";
        
        fs::path srcMeshPath = visualization->getFilename();
        assert(srcMeshPath.is_absolute());
        
        fs::path dstMeshFileName = srcMeshPath.filename();
        dstMeshFileName.replace_extension("stl");
        fs::path dstMeshRelPath = outputMeshRelDirectory / dstMeshFileName;
        fs::path dstMeshPath = outputDirectory / dstMeshRelPath;
        
        if (!fs::exists(dstMeshPath))
        {
            if (srcMeshPath.extension().string() != "stl")
            {
                std::cout << "Converting to .stl: " << srcMeshPath;
                
                if (!meshlabserverAviable)
                {
                    if (!notAvailableReported)
                    {
                        std::cout << std::endl 
                                  << "Command 'meshlabserver' not available, "
                                     "cannot convert meshes." << std::endl;
                        notAvailableReported = true;
                    }
                    continue;
                }
                
                // meshlabserver available
                std::stringstream convertCommand;
                convertCommand << "meshlabserver"
                               << " -i " << srcMeshPath.string() 
                               << " -o " << dstMeshPath.string();
                
                // run command
                int r = system(convertCommand.str().c_str());
                if (r != 0)
                {
                    std::cout << "Command returned with error: " << r << "\n"
                              << "Command was: " << convertCommand.str() << std::endl;
                }
            }
            else
            {
                std::cout << "Copying: " << srcMeshPath << "\n"
                          << "     to: " << dstMeshPath;
                fs::copy_file(srcMeshPath, dstMeshPath);
            }
        }
        else
        {
            std::cout << "skipping (" << dstMeshRelPath.string() << " already exists)";
        }
        std::cout << std::endl;
        
        
        
        // add asset
        std::string meshName = node->getName();
        document->addMeshElement(meshName, (outputDirectory/dstMeshRelPath).string());
        
        // add geom to body
        Element* body = nodeBodies[node->getName()];
        document->addGeomElement(body, meshName);
    }
}



Element* MujocoIO::addNodeBody(RobotNodePtr node)
{
    Element* element = nodeBodies[node->getName()];
    if (element)
    {
        // break recursion
        return element;
    }
    
    Element* parent = nodeBodies[node->getParent()->getName()];
    if (!parent)
    {
        parent = addNodeBody(robot->getRobotNode(node->getParent()->getName()));
    }
    
    element = document->addBodyElement(parent, node);
    nodeBodies[node->getName()] = element;

    return element;
}

void MujocoIO::addContactExcludes()
{
    RobotNodePtr rootNode = robot->getRootNode();
    
    std::vector<std::pair<std::string, std::string>> excludePairs;
            
    for (RobotNodePtr node : robot->getRobotNodes())
    {
        for (std::string& ignore : node->getPhysics().ignoreCollisions)
        {
            // I found an <IgnoreCollision> element referring to a non-existing node.
            // => check node existence here
            if (robot->hasRobotNode(ignore))
            {
                excludePairs.push_back({node->getName(), ignore});
            }
        }
    }
    
    // resolve body names and add exludes
    for (auto& excludePair : excludePairs)
    {
        std::string body1 = masslessBodySanitizer.getMergedBodyName(excludePair.first);
        std::string body2 = masslessBodySanitizer.getMergedBodyName(excludePair.second);
        document->addContactExclude(body1, body2);
    }
}

void MujocoIO::addActuators()
{
    std::vector<const mjcf::Element*> jointElements = getAllElements("joint");
    
    for (auto joint : jointElements)
    {
        std::string name = joint->Attribute("name");
        document->addMotorElement(name);
    }
}

struct ScaleLengthVisitor : public tx::XMLVisitor
{
    ScaleLengthVisitor(float scaling) : scaling(scaling) {}
    
    virtual bool VisitEnter(const tinyxml2::XMLElement& elem, const tinyxml2::XMLAttribute* attr) override;
    void applyScaling();
    
    float scaling;
    
    std::vector<const Element*> elementsToModify;
};

bool ScaleLengthVisitor::VisitEnter(const tinyxml2::XMLElement& elem, const tinyxml2::XMLAttribute* attr)
{
    if (isElement(elem, "joint"))
    {
        if (strcmp(elem.Attribute("type"), "hinge")==0
            && elem.Attribute("range"))
        {
            elementsToModify.push_back(&elem);
        }
    }
    else if (elem.Attribute("pos"))
    {
        elementsToModify.push_back(&elem);
    }
    
    return true;
}

void ScaleLengthVisitor::applyScaling()
{
    for (auto& elemConst : elementsToModify)
    {
        Element* elem = const_cast<Element*>(elemConst);
        if (isElement(elem, "joint"))
        {
            if (strcmp(elem->Attribute("type"), "hinge") == 0)
            {
                assert(elem->Attribute("range"));
                Eigen::Vector2f range = strToVec2(elem->Attribute("range"));
                range *= scaling;
                
                elem->SetAttribute("range", toAttr(range).c_str());
            }
        }
        else if (elemConst->Attribute("pos"))
        {
            Eigen::Vector3f pos = strToVec(elemConst->Attribute("pos"));
            pos *= scaling;
            elem->SetAttribute("pos", toAttr(pos).c_str());
        }
    }
}

void MujocoIO::scaleLengths()
{
    ScaleLengthVisitor visitor(lengthScaling);
    document->robotRootBody()->Accept(&visitor);
    visitor.applyScaling();
}

std::vector<const mjcf::Element*> MujocoIO::getAllElements(const std::string& elemName)
{
    mjcf::ListElementsVisitor visitor(elemName);
    document->worldbody()->Accept(&visitor);
    return visitor.getFoundElements();
}
