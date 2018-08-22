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
* @author     Harry
* @copyright  2017 Nikolaus Vahrenkamp
*             GNU Lesser General Public License
*
*/
#pragma once

#include "ModelNodeAttachment.h"

namespace VirtualRobot
{
    class ModelStructure : public ModelNodeAttachment
    {
        friend class ModelNode;
        friend class ModelStructureFactory;

    public:
        /*!
         * Constructor.
         * \param name  The name of the attachment.
         * \param localTransform    The transformation to apply to the attachment's pose after attaching to a ModelNode.
         */
        ModelStructure(const std::string &name, const Eigen::Matrix4f &localTransformation = Eigen::Matrix4f::Identity());

        virtual ~ModelStructure();

        /*!
         * Checks if this attachment is attachable to the given node.
         * Mostly determined on the basis of the node type.
         *
         * @param node The node to check, if this attachment is attachable.
         *
         * @return True, if this attachment is attachable; false otherwise.
         */
        virtual bool isAttachable(const ModelNodePtr &node) override;

        /*!
         * Get the type of this attachment.
         * This is used to separate different attached attachments.
         *
         * @return "ModelStructure".
         */
        virtual std::string getType() override;

        virtual ModelNodeAttachmentPtr clone() override;

        virtual std::string toXML(const std::string& basePath, const std::string& modelPathRelative = "models", int tabs = 3) override;

    private:
        void initVisualization();
        VisualizationPtr createJointVisualization(ModelJointPtr joint, VisualizationFactoryPtr factory);
        VisualizationPtr createLinkVisualization(ModelLinkPtr link, VisualizationFactoryPtr factory);

        // ModelNodeAttachment interface
    protected:
        virtual void setParent(const ModelNodePtr &node) override;
    };

    typedef std::shared_ptr<ModelStructure> ModelStructurePtr;
}
