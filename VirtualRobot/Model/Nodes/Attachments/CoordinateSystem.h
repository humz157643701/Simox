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
#pragma once

#include "CustomVisualizationAttachment.h"

#include <cstdint>
#include <string>

namespace VirtualRobot
{
    class CoordinateSystem : public CustomVisualizationAttachment
    {
        friend class CoordinateSystemFactory;

    protected:
        /*!
         * Constructor.
         * \param name  The name of the attachment.
         * \param localTransform    The transformation to apply to the attachment's pose after attaching to a ModelNode.
         */
        CoordinateSystem(const std::string &name, const Eigen::Matrix4f &localTransformation = Eigen::Matrix4f::Identity());

    public:
        /*!
         * Destructor.
         */
        virtual ~CoordinateSystem() override;

        /*!
         * Get the type of this attachment.
         * This is used to seperate different attached attachments.
         *
         * @return The type of this attachment.
         */
        virtual std::string getType() const override;

        virtual ModelNodeAttachmentPtr clone() const override;
    };
    using CoordinateSystemPtr = std::shared_ptr<CoordinateSystem>;
}