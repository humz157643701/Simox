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
* @copyright  2011 Nikolaus Vahrenkamp
*             GNU Lesser General Public License
*
*/
#pragma once

#include "../GraspPlanning.h"
#include <VirtualRobot/VirtualRobot.h>
#include <VirtualRobot/Tools/MathTools.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace GraspPlanning
{
    /*!
     *
     * \brief A visualization of a convex hull
     * @see CoinConvexHullVisualization
     *
     */
    class GRASPPLANNING_IMPORT_EXPORT ConvexHullVisualization
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        /*!
            Constructor
        */
        ConvexHullVisualization(VirtualRobot::MathTools::ConvexHull6DPtr convHull, bool useFirst3Coords = true);
        ConvexHullVisualization(VirtualRobot::MathTools::ConvexHull3DPtr convHull);

        ~ConvexHullVisualization() = default;

        virtual VirtualRobot::VisualizationPtr getVisualization();

    protected:
        static VirtualRobot::VisualizationPtr createConvexHullVisualization(const VirtualRobot::MathTools::ConvexHull3DPtr& convHull);
        static VirtualRobot::VisualizationPtr createConvexHullVisualization(const VirtualRobot::MathTools::ConvexHull6DPtr& convHull, bool buseFirst3Coords);

        VirtualRobot::MathTools::ConvexHull3DPtr convHull3D;
        VirtualRobot::MathTools::ConvexHull6DPtr convHull6D;
        bool useFirst3Coords;

        VirtualRobot::VisualizationPtr visualization;
    };

    typedef std::shared_ptr<ConvexHullVisualization> ConvexHullVisualizationPtr;
} // namespace GraspPlanning
