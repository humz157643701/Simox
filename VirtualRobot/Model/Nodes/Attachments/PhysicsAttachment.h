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

#include "CustomVisualizationAttachment.h"

namespace VirtualRobot
{
    /**
     * An attachment visualizing the CoM and/or the inertia tensor when attached to a ModelLink.
     */
    class PhysicsAttachment : public CustomVisualizationAttachment
    {
        friend class PhysicsAttachmentFactory;

    public:
        PhysicsAttachment(const std::string &name, const Eigen::Matrix4f &localTransformation);

        /*!
         * Get the type of this attachment.
         * This is used to separate different attached attachments.
         *
         * @return "PhysicsAttachment".
         */
        virtual std::string getType() const override;
        virtual ModelNodeAttachmentPtr clone() const override;

        void enableVisualization(bool CoM, bool inertia);

    protected:
        void setParent(const ModelNodePtr &node) override;

    private:
        void initVisualization();
    };

    typedef std::shared_ptr<PhysicsAttachment> PhysicsAttachmentPtr;
}
