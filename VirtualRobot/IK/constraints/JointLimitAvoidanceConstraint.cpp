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
* @author     Peter Kaiser
* @copyright  2016 Peter Kaiser
*             GNU Lesser General Public License
*
*/

#include "JointLimitAvoidanceConstraint.h"

using namespace VirtualRobot;

JointLimitAvoidanceConstraint::JointLimitAvoidanceConstraint(const RobotPtr &robot, const JointSetPtr &nodeSet) :
    ReferenceConfigurationConstraint(robot, nodeSet)
{
    Eigen::VectorXf reference(nodeSet->getSize());

    for(unsigned int i = 0; i < nodeSet->getSize(); i++)
    {
        ModelJointPtr node = nodeSet->getNode(i);
        reference(i) = node->getJointLimitLow() + (node->getJointLimitHigh() - node->getJointLimitLow()) / 2;
    }

    setReferenceConfiguration(reference);
}
