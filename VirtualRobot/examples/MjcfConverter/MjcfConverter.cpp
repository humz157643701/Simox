#include "MjcfConverter.h"

#include <VirtualRobot/RobotNodeSet.h>
#include <VirtualRobot/XML/RobotIO.h>

#include "utils.h"


using namespace VirtualRobot;
namespace tx = tinyxml2; 
namespace fs = boost::filesystem;
using Element = mjcf::Element;


MjcfConverter::MjcfConverter()
{
}

void MjcfConverter::convert(const std::string& inputSimoxXmlFile, 
                            const std::string& outputDirectory)
{
    setPaths(inputSimoxXmlFile, outputDirectory);
    
    loadInputFile();
    convertToMjcf();
    writeOutputFile();
}

void MjcfConverter::setPaths(const std::string& inputFilename, 
                             const std::string& outputDirectory)
{
    this->inputFilePath = inputFilename;
    
    inputFileDirectory = inputFilePath.parent_path();
    inputFileName = inputFilePath.filename();
    
    this->outputDirectory = outputDirectory;
    outputFileName = this->outputDirectory / inputFileName;
    
    outputMeshRelDirectory = "mesh";
    
    
    auto ensureDirExists = [](const fs::path& path)
    {
        if (!fs::exists(path))
        {
            fs::create_directory(path);
        }
    };
    
    ensureDirExists(outputDirectory);
    ensureDirExists(outputDirectory / outputMeshRelDirectory);
    
    assert(!inputFileDirectory.empty());
}

void MjcfConverter::makeEnvironment()
{
    document->addSkyboxTexture(Eigen::Vector3f(.8f, .9f, .95f), 
                               Eigen::Vector3f(.4f, .6f, .8f));
}

void MjcfConverter::loadInputFile()
{
    assert(!inputFilePath.empty());
    
    try
    {
        this->robot = RobotIO::loadRobot(inputFilePath.string(), RobotIO::eStructure);
        assert(robot);
    }
    catch (const VirtualRobotException&)
    {
        throw; // rethrow
    }
    
    inputXML.LoadFile(inputFilePath.string());
}

void MjcfConverter::writeOutputFile()
{
    assert(!outputFileName.empty());
    
    VR_INFO << std::endl;
    document->Print();
    
    VR_INFO << "Writing to " << outputFileName << std::endl;
    document->SaveFile(outputFileName.c_str());
}


void MjcfConverter::convertToMjcf()
{
    document.reset(new mjcf::Document());
    
    document->setModelName(robot->getName());
    document->compiler()->SetAttribute("angle", "radian");
    
    makeEnvironment();
    
    VR_INFO << "Creating bodies structure ..." << std::endl;
    addNodeBodies();
    
    VR_INFO << "Adding meshes and geoms ..." << std::endl;
    addNodeBodyMeshes();
    
    VR_INFO << "===========================" << std::endl
              << "Current model: "             << std::endl
              << "--------------"              << std::endl;
    document->Print();
    VR_INFO << "===========================" << std::endl;
    
    VR_INFO << "Merging empty bodies ..." << std::endl;
    sanitizeMasslessBodies();
    
    VR_INFO << "Adding contact excludes ..." << std::endl;
    document->addContactExcludes(nodeBodies[robot->getRootNode()->getName()]);
    
    VR_INFO << "Done.";
    
    return; 
}



void MjcfConverter::addNodeBodies()
{
    nodeBodies.clear();
    
    RobotNodePtr rootNode = robot->getRootNode();
    assert(rootNode);
    
    // add root
    Element* root = document->addBodyElement(document->worldbody(), rootNode);
    nodeBodies[rootNode->getName()] = root;
    assert(root);
    
    for (RobotNodePtr node : robot->getRobotNodes())
    {
        addNodeBody(node);
    }
}

void MjcfConverter::addNodeBodyMeshes()
{
    bool meshlabserverAviable = system("which meshlabserver > /dev/null 2>&1") == 0;
    bool notAvailableReported = false;
    
    for (RobotNodePtr node : robot->getRobotNodes())
    {
        if (!inputXML.hasVisualizationFile(node))
        {
            continue;
        }
        
        VR_INFO << "Node " << node->getName() << ":\t";
        
        fs::path srcMeshPath = inputXML.visualizationFile(node);
        
        if (srcMeshPath.is_relative())
        {
            // resolve relative path
            srcMeshPath = inputFileDirectory / srcMeshPath;
        }
        
        fs::path dstMeshFileName = srcMeshPath.filename();
        dstMeshFileName.replace_extension("stl");
        fs::path dstMeshRelPath = outputMeshRelDirectory / dstMeshFileName;
        fs::path dstMeshPath = outputDirectory / dstMeshRelPath;
        
        if (!fs::exists(dstMeshPath))
        {
            if (srcMeshPath.extension().string() != "stl")
            {
                VR_INFO << "Converting to .stl: " << srcMeshPath;
                
                if (!meshlabserverAviable)
                {
                    if (!notAvailableReported)
                    {
                        VR_INFO << std::endl 
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
                    VR_INFO << "Command returned with error: " << r << "\n"
                              << "Command was: " << convertCommand.str() << std::endl;
                }
            }
            else
            {
                VR_INFO << "Copying: " << srcMeshPath << "\n"
                          << "     to: " << dstMeshPath;
                fs::copy_file(srcMeshPath, dstMeshPath);
            }
        }
        else
        {
            VR_INFO << "skipping (" << dstMeshRelPath.string() << " already exists)";
        }
        VR_INFO << std::endl;
        
        
        
        // add asset
        std::string meshName = node->getName();
        document->addMeshElement(meshName, dstMeshRelPath.string());
        
        // add geom to body
        Element* body = nodeBodies[node->getName()];
        document->addGeomElement(body, meshName);
    }
}



Element* MjcfConverter::addNodeBody(RobotNodePtr node)
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

void MjcfConverter::sanitizeMasslessBodies()
{
    // merge body leaf nodes with parent if they do not have a geom
    
    // assume we have leaf
    
    Element* root = document->worldbody()->FirstChildElement("body");
    
    for (Element* body = root->FirstChildElement("body");
         body;
         body = body->NextSiblingElement("body"))
    {
        sanitizeMasslessBodyRecursion(body);
    }
}

void MjcfConverter::sanitizeMasslessBodyRecursion(mjcf::Element* body)
{
    assertElementIsBody(body);
    
    VR_INFO << "- Node '" << body->Attribute("name") << "': " << std::endl;
    
    // leaf => end of recursion
    if (!hasElementChild(body, "body"))
    {
        VR_INFO << "  | Leaf";
        if (!hasMass(body))
        {
            VR_INFO << " without mass" << std::endl;
            sanitizeMasslessLeafBody(body);
        }
        else
        {
            VR_INFO << std::endl;
        }
        return; 
    }
    
    // non-leaf body
    VR_INFO << "  | Non-leaf" << std::endl;
    
    std::string bodyName = body->Attribute("name");
    RobotNodePtr bodyNode = robot->getRobotNode(bodyName);
    Eigen::Matrix4f bodyPose = bodyNode->getTransformationFrom(bodyNode->getParent());
    
    while (!hasMass(body))
    {
        VR_INFO << "  | No mass" << std::endl;
        
        // check whether there is only one child body
        Element* childBody = body->FirstChildElement("body");
        if (!childBody->NextSiblingElement("body"))
        {
            std::string childBodyName = childBody->Attribute("name");
            
            VR_INFO << "  | Single child body => merging '" << childBodyName
                      << "' into '" << bodyName << "'" << std::endl;

            // check child for pose attributes
            if (childBody->Attribute("pos") || childBody->Attribute("quat"))
            {
                // update body's transform and joint axes
                
                RobotNodePtr childNode = robot->getRobotNode(childBodyName);
                Eigen::Matrix4f childPose = childNode->getTransformationFrom(childNode->getParent());
                
                // body's pose
                bodyPose = bodyPose * childPose;
                document->setBodyPose(body, bodyPose);
                
                /* Adapt axes of joints in body:
                 * The axes will be relative to the new pose. Therefore, the additional rotation
                 * of the child must be subtracted from the joint axis.
                 */
                
                Eigen::Matrix3f revChildOri = childPose.block<3,3>(0, 0).transpose();
                
                for (Element* joint = body->FirstChildElement("joint"); joint;
                     joint = joint->NextSiblingElement("joint"))
                {
                    Eigen::Vector3f axis = strToVec(joint->Attribute("axis"));
                    // apply child orientation
                    axis = revChildOri * axis;
                    document->setJointAxis(joint, axis);
                }
            }
            
            
            // merge childBody into body => move all its elements here
            for (tx::XMLNode* grandChild = childBody->FirstChild(); grandChild;
                 grandChild = grandChild->NextSibling())
            {
                VR_INFO << "  |  | Moving '" << grandChild->Value() << "'" << std::endl;
                
                // clone grandchild
                tx::XMLNode* grandChildClone = grandChild->DeepClone(nullptr);
                
                // insert to body
                body->InsertEndChild(grandChildClone);
            }
            
            
            
            // update body name
            
            std::stringstream newName;
            /*
            std::size_t prefixSize = commonPrefixLength(bodyName, childBodyName);
            newName << bodyName.substr(0, prefixSize)
                    << bodyName.substr(prefixSize) << "~" << childBodyName.substr(prefixSize);
            */
            newName << bodyName << "~" << childBodyName;
            body->SetAttribute("name", newName.str().c_str());
            
            
            // delete child
            body->DeleteChild(childBody);
        }
        else
        {
            VR_WARNING << "  | Massless body with >1 child body: " 
                       << body->Attribute("name") << std::endl;
            break;
        }
    }
    
    for (Element* child = body->FirstChildElement("body");
         child;
         child = child->NextSiblingElement("body"))
    {
        sanitizeMasslessBodyRecursion(child);
    }
    
}

void MjcfConverter::sanitizeMasslessLeafBody(mjcf::Element* body)
{
    
    assert(!hasElementChild(body, "body"));
    assert(!hasMass(body));
    
    if (body->NoChildren()) // is completely empty?
    {
        // leaf without geom: make it a site
        VR_INFO << "  | Empty => Changing body '" << body->Attribute("name") << "' to site." << std::endl;
        body->SetValue("site");
    }
    else
    {
        // add a small mass
        VR_INFO << "  | Not empty => Adding dummy inertial." << std::endl;
        document->addDummyInertial(body);
    }
}





