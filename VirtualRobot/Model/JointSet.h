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
* @author     Adrian Knobloch
* @copyright  2016 Adrian Knobloch
*             GNU Lesser General Public License
*
*/
#ifndef _VirtualRobot_JointSet_h_
#define _VirtualRobot_JointSet_h_

#include "../Model/Model.h"
#include "ModelNodeSet.h"
#include "Nodes/ModelJoint.h"

namespace VirtualRobot
{
    class VIRTUAL_ROBOT_IMPORT_EXPORT JointSet : public ModelNodeSet
    {
    public:
        /*!
         * Create a new JointSet.
         *
         * @param model The associated model.
         * @param name The name of the new JointSet.
         * @param modelNodeNames The names of the model nodes to add.
         * @param kinematicRootName The name of the kinematic root.
         *                          This specifies the first node of the model's kinematic tree to be used for updating all members of this set.
         *                          The kinematic root does not have to be a node of this set.
         *                          If no name provided, the first node of the given model nodes will be set as the kinematic root.
         * @param tcpName The name of the tcp.
         *                The tcp does not have to be a node of this set.
         *                If no name provided, the last node of the given model nodes will be set as the tcp node.
         * @param registerToModel If true, the new JointSet is registered to the model.
         * @return The newly created JointSet.
         */
        static JointSetPtr createJointSet(const ModelPtr& model,
                                                  const std::string& name,
                                                  const std::vector<std::string>& jointNames,
                                                  const std::string& kinematicRootName = "",
                                                  const std::string& tcpName = "",
                                                  bool registerToModel = false);
        /*!
         * Create a new JointSet.
         *
         * @param model The associated model.
         * @param name The name of the new JointSet.
         * @param modelNodes The nodes to add to this set.
         * @param kinematicRoot This specifies the first node of the model's kinematic tree to be used for updating all members of this set.
         *                      The kinematic root does not have to be a node of this set.
         *                      If no kinematic root provided, the first node of the given model nodes will be set as the kinematic root.
         * @param tcp The tcp.
         *            The tcp does not have to be a node of this set.
         *            If no tcp provided, the last node of the given model nodes will be set as the tcp node.
         * @param registerToModel If true, the new JointSet is registered to the model.
         * @return The newly created JointSet.
         */
        static JointSetPtr createJointSet(const ModelPtr& model,
                                                  const std::string& name,
                                                  const std::vector<ModelNodePtr> &modelNodes,
                                                  const ModelNodePtr kinematicRoot = ModelNodePtr(),
                                                  const FramePtr tcp = FramePtr(),
                                                  bool registerToModel = false);
        static JointSetPtr createJointSet(const ModelPtr& model,
                                                  const std::string& name,
                                                  const std::vector<ModelJointPtr> &modelNodes,
                                                  const ModelNodePtr kinematicRoot = ModelNodePtr(),
                                                  const FramePtr tcp = FramePtr(),
                                                  bool registerToModel = false);

    protected:
        /*!
         * Initialize this set with a vector of ModelNodes.
         *
         * @param name The name of this JointSet.
         * @param model The associated model.
         * @param modelNodes The model nodes to add to this JointSet.
         * @param kinematicRoot    This specifies the first node of the model's kinematic tree to be used for updating all members of this set.
         *                         kinematicRoot does not have to be a node of this set.
         *                         If not given, the first entry of modelNodes will be set as the kinematic root.
         * @param tcp   The tcp.
         *              If not given, the last entry of modelNodes will be set as the tcp.
         */
        JointSet(const std::string& name,
                     const ModelWeakPtr& model,
                     const std::vector<ModelJointPtr> &jointNodes,
                     const ModelNodePtr kinematicRoot = ModelNodePtr(),
                     const FramePtr tcp = FramePtr());

    public:
        /*!
         * Destructor.
         */
        virtual ~JointSet();

        virtual ModelNodePtr getNode(size_t i) const override;
        ModelJointPtr getJoint(size_t i) const;

        virtual bool hasNode(const ModelNodePtr &node) const override;
        virtual bool hasNode(const std::string &nodeName) const override;
        inline bool hasJoint(const ModelJointPtr &joint) const
        {
            return hasNode(joint);
        }
        inline bool hasJoint(const std::string &linkName) const
        {
            return hasNode(linkName);
        }

        virtual std::vector<ModelNodePtr> getNodes() const override;
        virtual std::vector<ModelJointPtr> getJoints() const override;
        virtual std::vector<ModelLinkPtr> getLinks() const override;

        virtual unsigned int getSize() const override;

        virtual ModelNodePtr getKinematicRoot() const override;
        virtual void setKinematicRoot(const ModelNodePtr &modelNode) override;

        virtual FramePtr getTCP() const override;

        virtual void print() const override;
        virtual std::string toXML(int tabs) const override;

        virtual ModelNodeSetPtr clone(const ModelPtr& model, const std::string& newName = "", bool registerToModel = true) const override;

        /*!
         * Get the joint values of all contained joints.
         *
         * @return The joint values.
         */
        std::vector<float> getJointValues() const;

        /*!
         * Get the joint values of all contained joints.
         *
         * @param fillVector The vector to put the values in.
         * @param clearVector If true, all items in the vector are deleted.
         */
        void getJointValues(std::vector<float>& fillVector, bool clearVector = false) const;

        /*!
         * Get the joint values of all contained joints.
         *
         * @param fillVector The vector to put the values in.
         */
        void getJointValues(Eigen::VectorXf& fillVector) const;

        /*!
         * Get the joint values of all contained joints.
         *
         * @param config The config to save the joint values in.
         */
        void getJointValues(const ModelConfigPtr& config) const;

        /*!
         * Checks if the given joint values are within joint limits.
         * If not the joint values are adjusted.
         *
         * @param jointValues The values to check and adjust.
         */
        void respectJointLimits(std::vector<float>& jointValues) const;

        /*!
         * Checks if the given joint values are within joint limits.
         * If not the joint values are adjusted.
         *
         * @param jointValues The values to check and adjust.
         */
        void respectJointLimits(Eigen::VectorXf& jointValues) const;

        /*!
         * Checks if the jointValues are within the current joint limits.
         *
         * @param jointValues A vector of correct size.
         * @param verbose Print information if joint limits are violated.
         * @return True when all given joint values are within joint limits.
         */
        bool checkJointLimits(const std::vector<float>& jointValues, bool verbose = false) const;

        /*!
         * Checks if the jointValues are within the current joint limits.
         *
         * @param jointValues A vector of correct size.
         * @param verbose Print information if joint limits are violated.
         * @return True when all given joint values are within joint limits.
         */
        bool checkJointLimits(const Eigen::VectorXf& jointValues, bool verbose = false) const;

        /*!
         * Set joint values [rad].
         * The subpart of the robot, defined by the start joint (kinematicRoot) of rns, is updated to apply the new joint values.
         *
         * @param jointValues A vector with joint values, size must be equal to number of joints in this RobotNodeSet.
         */
        void setJointValues(const std::vector<float>& jointValues);

        /*!
         * Set joint values [rad].
         * The subpart of the robot, defined by the start joint (kinematicRoot) of rns, is updated to apply the new joint values.
         *
         * @param jointValues A vector with joint values, size must be equal to number of joints in this RobotNodeSet.
        */
        void setJointValues(const Eigen::VectorXf& jointValues);

        /*!
         * Set joints that are within the given ModelConfig. Joints of this NodeSet that are not stored in jointValues remain untouched.
         *
         * @param config The config to get the joint values from.
         */
        void setJointValues(const ModelConfigPtr& config);
        
        std::map< std::string, float > getJointValueMap() const;

    private:
        std::vector<ModelJointPtr> joints;
        ModelNodePtr kinematicRoot;
        FramePtr tcp;
    };
}

#endif // _VirtualRobot_JointSet_h_
