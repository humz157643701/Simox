/**
* This file is part of Simox.
*
* Simox is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* Simox is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* @package    VirtualRobot
* @author     Nikolaus Vahrenkamp
* @copyright  2011 Nikolaus Vahrenkamp
*             GNU Lesser General Public License
*
*/
#ifndef _VirtualRobot_BasicDefinitions_h_
#define _VirtualRobot_BasicDefinitions_h_

#include "VirtualRobotImportExport.h"

/*! \defgroup VirtualRobot The VirtualRobot Library
* With the VirtualRobot library you can define complex robot structures,
* perform collision detection, visualize robots and environments, do reachability analysis and generic IK solvers are provided.
*/



/** \mainpage Simox: A simulation, motion and grasp planning toolbox.

  \section Introduction Introduction

  The aim of the lightweight platform independent C++ toolbox \b Simox is to provide a set of
  algorithms for 3D simulation of robot systems, sampling based motion planning and grasp
  planning. Simox consists of three libraries (VirtualRobot, Saba and GraspStudio) and numerous
  examples showing how these libraries can be used to build complex tools in the
  context of mobile manipulation.

  Further information and documentation can be found at the wiki pages: http://sourceforge.net/p/simox/wiki/

  \section VirtualRobot VirtualRobot

  The library \b VirtualRobot can be used to define complex
  robot systems, which may cover multiple robots with many degrees of freedom. The robot
  structure and its visualization can be easily defined via XML files and environments with
  obstacles and objects to manipulate are supported. Further, basic robot simulation components,
  as Jacobian computations and generic Inverse Kinematics (IK) solvers, are offered by
  the library. Beyond that, extended features like tools for analyzing the reachable workspace
  for robotic manipulators or contact determination for grasping are included.
  \image html VR.png

  \section Saba Motion Planning

  With \b Saba, a library for planning collision-free motions is offered, which directly incorporates
  with the data provided by VirtualRobot. The algorithms cover state-of-the-art implementations
  of sampling-based motion planning approaches (e.g. Rapidly-exploring Random Trees)
  and interfaces that allow to conveniently implement own planners. Since Saba was designed
  for planning in high-dimensional configuration spaces, complex planning problems for robots
  with a high number of degrees of freedom (DoF) can be solved efficiently.

  \image html Saba.png

  \section GraspStudio Grasp Planning

  \b GraspStudio offers possibilities to compute the grasp quality for generic end-effector definitions,
  e.g. a humanoid hand. The implemented 6D wrench-space computations can be used
  to easily (and quickly) determine the quality of an applied grasp to an object. Furthermore,
  the implemented planners are able to generate grasp maps for given objects automatically.

  \image html GraspStudio.png

  \section Wiki Installation, tutorials and documentation

  Since complex frameworks have to incorporate with several libraries in order to provide full
  functionality, several issues may arise when setting up the environment, such as dependency
  problems, incompatible library versions or even non-existing ports of needed libraries for the
  used operating systems. Hence, only a limited set of libraries are used by the Simox core in
  order to make it compile. Extended functionality (e.g. visualization) can be turned off in
  order to allow Simox compiling on most platforms. Further dependencies are encapsulated
  with interfaces, making it easy to exchange e.g. the collision engine or the visualization
  functionality. As a reference implementation Simox offers Coin3D/SoQt-based visualization
  support.

  Please have a look at the wiki pages: http://sourceforge.net/p/simox/wiki/
 *
 */

// include compile time defines, generated by cmake
//#include "definesVR.h"
// for now we know that PQP is used- ToDo: Change CollisionChecker implementation, use AbstractFactoryMethods
#define VR_COLLISION_DETECTION_PQP


#ifdef WIN32
// needed to have M_PI etc defined
#if !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif

#endif


// allow std vector to be used with Eigen objects

#include<Eigen/StdVector>
#ifndef EIGEN_STL_VECTOR_SPECIFICATION_DEFINED
#define EIGEN_STL_VECTOR_SPECIFICATION_DEFINED
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector2f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector3f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector4f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::VectorXf)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix2f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix4f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::MatrixXf)

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector3d)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector3i)
#endif

#ifndef Q_MOC_RUN // workaround for some bug in some QT/boost versions
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/mem_fn.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/version.hpp>
#include <boost/format.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/current_function.hpp>
#endif


#include <iostream>
#include <sstream>
#include <cmath>

//#ifdef _DEBUG
//#ifdef WIN32
// ENABLE MEMORY LEAK CHECKING FOR WINDOWS
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#endif
//#endif

namespace VirtualRobot
{

    // only valid within the VirtualRobot namespace
    using std::cout;
    using std::endl;

    class CoMIK;
    class DifferentialIK;
    class HierarchicalIK;
    class Constraint;
    class TSRConstraint;
    class BalanceConstraint;
    class PoseConstraint;
    class PositionConstraint;
    class OrientationConstraint;
    class SupportPolygon;
    class KinematicChain;
    class EndEffector;
    class EndEffectorActor;
    class CollisionChecker;
    class CollisionModel;
    class TriMeshModel;
    class Obstacle;
    class Visualization;
    class VisualizationNode;
    class VisualizationFactory;
    class Scene;
    class ModelConfig;
    class Grasp;
    class GraspSet;
    class ManipulationObject;
    class CDManager;
    class Reachability;
    class WorkspaceRepresentation;
    class WorkspaceData;
    class PoseQualityMeasurement;
    class PoseQualityManipulability;
    class Trajectory;
    class SphereApproximator;
    class BasicGraspQualityMeasure;
    class WorkspaceGrid;
    class WorkspaceDataArray;
    class Model;
    class ModelNode;
    class ModelLink;
    class ModelJoint;
    class ModelJointFixed;
    class ModelJointPrismatic;
    class ModelJointRevolute;
    class ModelNodeSet;
    class LinkSet;
    class JointSet;
    class ModelNodeAttachment;
    class ModelNodeAttachmentFactory;
    typedef Model Robot; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelNode RobotNode; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelLink RobotLink; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelJoint RobotJoint; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelJointFixed RobotJointFixed; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelJointPrismatic RobotJointPrismatic; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelJointRevolute RobotJointRevolute; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelNodeSet RobotNodeSet; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelNodeAttachment RobotNodeAttachment; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelNodeAttachmentFactory RobotNodeAttachmentFactory; //!< A typedef to make a differ between a robot and a simple object
    typedef ModelConfig RobotConfig; //!< A typedef to make a differ between a robot and a simple object

    typedef boost::shared_ptr<CoMIK> CoMIKPtr;
    typedef boost::shared_ptr<HierarchicalIK> HierarchicalIKPtr;
    typedef boost::shared_ptr<DifferentialIK> DifferentialIKPtr;
    typedef boost::shared_ptr<Constraint> ConstraintPtr;
    typedef boost::shared_ptr<TSRConstraint> TSRConstraintPtr;
    typedef boost::shared_ptr<BalanceConstraint> BalanceConstraintPtr;
    typedef boost::shared_ptr<PoseConstraint> PoseConstraintPtr;
    typedef boost::shared_ptr<PositionConstraint> PositionConstraintPtr;
    typedef boost::shared_ptr<OrientationConstraint> OrientationConstraintPtr;
    typedef boost::shared_ptr<SupportPolygon> SupportPolygonPtr;
    typedef boost::shared_ptr<KinematicChain> KinematicChainPtr;
    typedef boost::shared_ptr<EndEffector> EndEffectorPtr;
    typedef boost::shared_ptr<EndEffectorActor> EndEffectorActorPtr;
    typedef boost::shared_ptr<CollisionModel> CollisionModelPtr;
    typedef boost::shared_ptr<CollisionChecker> CollisionCheckerPtr;
    typedef boost::shared_ptr<TriMeshModel> TriMeshModelPtr;
    typedef boost::shared_ptr<Obstacle> ObstaclePtr;
    typedef boost::shared_ptr<Visualization> VisualizationPtr;
    typedef boost::shared_ptr<VisualizationNode> VisualizationNodePtr;
    typedef boost::shared_ptr<VisualizationFactory> VisualizationFactoryPtr;
    typedef boost::shared_ptr<WorkspaceData> WorkspaceDataPtr;
    typedef boost::shared_ptr<WorkspaceDataArray> WorkspaceDataArrayPtr;
    typedef boost::shared_ptr<WorkspaceRepresentation> WorkspaceRepresentationPtr;
    typedef boost::shared_ptr<Reachability> ReachabilityPtr;
    typedef boost::shared_ptr<Scene> ScenePtr;
    typedef boost::shared_ptr<ModelConfig> ModelConfigPtr;
    typedef boost::shared_ptr<Grasp> GraspPtr;
    typedef boost::shared_ptr<GraspSet> GraspSetPtr;
    typedef boost::shared_ptr<ManipulationObject> ManipulationObjectPtr;
    typedef boost::shared_ptr<CDManager> CDManagerPtr;
    typedef boost::shared_ptr<PoseQualityMeasurement> PoseQualityMeasurementPtr;
    typedef boost::shared_ptr<PoseQualityManipulability> PoseQualityManipulabilityPtr;
    typedef boost::shared_ptr<Trajectory> TrajectoryPtr;
    typedef boost::shared_ptr<SphereApproximator> SphereApproximatorPtr;
    typedef boost::shared_ptr<BasicGraspQualityMeasure> BasicGraspQualityMeasurePtr;
    typedef boost::shared_ptr<WorkspaceGrid> WorkspaceGridPtr;
    typedef boost::shared_ptr<Model> ModelPtr;
    typedef boost::weak_ptr<Model> ModelWeakPtr;
    typedef boost::shared_ptr<ModelNode> ModelNodePtr;
    typedef boost::weak_ptr<ModelNode> ModelNodeWeakPtr;
    typedef boost::shared_ptr<ModelLink> ModelLinkPtr;
    typedef boost::shared_ptr<ModelJoint> ModelJointPtr;
    typedef boost::shared_ptr<ModelJointFixed> ModelJointFixedPtr;
    typedef boost::shared_ptr<ModelJointPrismatic> ModelJointPrismaticPtr;
    typedef boost::shared_ptr<ModelJointRevolute> ModelJointRevolutePtr;
    typedef boost::shared_ptr<ModelNodeSet> ModelNodeSetPtr;
    typedef boost::shared_ptr<LinkSet> LinkSetPtr;
    typedef boost::shared_ptr<JointSet> JointSetPtr;
    typedef boost::shared_ptr<ModelNodeAttachment> ModelNodeAttachmentPtr;
    typedef boost::shared_ptr<ModelNodeAttachmentFactory> ModelNodeAttachmentFactoryPtr;
    typedef boost::shared_ptr<Robot> RobotPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::weak_ptr<Robot> RobotWeakPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotNode> RobotNodePtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::weak_ptr<RobotNode> RobotNodeWeakPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotLink> RobotLinkPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotJoint> RobotJointPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotJointFixed> RobotJointFixedPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotJointPrismatic> RobotJointPrismaticPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotJointRevolute> RobotJointRevolutePtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotNodeSet> RobotNodeSetPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotNodeAttachment> RobotNodeAttachmentPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotNodeAttachmentFactory> RobotNodeAttachmentFactoryPtr; //!< A typedef to make a differ between a robot and a simple object
    typedef boost::shared_ptr<RobotConfig> RobotConfigPtr; //!< A typedef to make a differ between a robot and a simple object

    /*
     * Predefine for MathTools.h
     */
    namespace MathTools
    {
        struct Quaternion;
        struct SphericalCoord;
        struct Segment2D;
        struct ConvexHull2D;
        struct ConvexHull3D;
        struct ConvexHull6D;
        struct Plane;
        template<typename VectorT> struct BaseLine;
        struct Segment;
        struct OOBB;
        struct ContactPoint;
        struct TriangleFace;
        struct TriangleFace6D;

        typedef BaseLine<Eigen::Vector3f> Line;
        typedef BaseLine<Eigen::Vector2f> Line2D;
        typedef boost::shared_ptr<ConvexHull2D> ConvexHull2DPtr;
        typedef boost::shared_ptr<ConvexHull3D> ConvexHull3DPtr;
        typedef boost::shared_ptr<ConvexHull6D> ConvexHull6DPtr;

   }


#define VR_INFO std::cout <<__FILE__ << ":" << __LINE__ << ": "
#define VR_WARNING std::cerr <<__FILE__ << ":" << __LINE__ << " -Warning- "
#define VR_ERROR std::cerr <<__FILE__ << ":" << __LINE__ << " - ERROR - "


#ifdef NDEBUG

#define VR_ASSERT(a)
#define VR_ASSERT_MESSAGE(a,b)

#else
	/*!
	This assert macro does nothing on RELEASE builds.
	*/
#define VR_ASSERT( a )  BOOST_ASSERT( a )
	//THROW_VR_EXCEPTION_IF(!(a), "ASSERT failed (" << #a << ")" );

	// we have to switch to boost 1.48 to allow messages (BOOST_ASSERT_MSG) ....
#define VR_ASSERT_MESSAGE(a,b) BOOST_ASSERT(a)
	//THROW_VR_EXCEPTION_IF(!(a), "ASSERT failed (" << #a << "): " << b );

#endif


    /*!
    Initialize the runtime envionment. This method calls VisualizationFactory::init().
    */
    void VIRTUAL_ROBOT_IMPORT_EXPORT init(int &argc, char* argv[], const std::string &appName);
    void VIRTUAL_ROBOT_IMPORT_EXPORT init(const std::string &appName);

    // init method is storing appName, since the c_string is passed by refrence to QT -> we must ensure that the string stays alive
    VIRTUAL_ROBOT_IMPORT_EXPORT extern std::string globalAppName;

} // namespace

#endif // _VirtualRobot_BasicDefinitions_h_
