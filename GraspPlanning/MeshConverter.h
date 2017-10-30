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
* @package    GraspPlanning
* @author     Nikolaus Vahrenkamp
* @copyright  2013 H2T,KIT
*             GNU Lesser General Public License
*
*/
#ifndef _GRASP_PLANNING_MESHCONVERTER_H
#define _GRASP_PLANNING_MESHCONVERTER_H

#include "GraspPlanning.h"
#include <vector>
#include <VirtualRobot/Model/ManipulationObject.h>
#include <VirtualRobot/Visualization/TriMeshModel.h>

namespace GraspPlanning
{

    class GRASPPLANNING_IMPORT_EXPORT MeshConverter
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        /*!
            Create an object. The visualization and collision model is created from the convex hull.
        */
        static VirtualRobot::ManipulationObjectPtr CreateManipulationObject(const std::string& name, const VirtualRobot::MathTools::ConvexHull3DPtr &hull);
        static VirtualRobot::TriMeshModelPtr CreateTriMeshModel(const VirtualRobot::MathTools::ConvexHull3DPtr &hull);
        static VirtualRobot::ObstaclePtr RefineObjectSurface(const VirtualRobot::ObstaclePtr &object, float maxDist);

        //! Returns -1 if obj is not part of vectList, otherwise the index of vectList is returned.
        static int hasVertex(const std::vector< Eigen::Vector3f>& vectList, const Eigen::Vector3f& obj);
        static void checkAndSplitVertex(const VirtualRobot::TriMeshModelPtr &tm, int faceIdx, float maxDist);

        static float getMaxVertexDistance(const VirtualRobot::TriMeshModelPtr &tm);

    };
}
#endif // MESHCONVERTER_H
