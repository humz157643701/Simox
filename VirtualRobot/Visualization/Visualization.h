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
* @author     Manfred Kroehnert, Nikolaus Vahrenkamp
* @copyright  2010,2011 Manfred Kroehnert, Nikolaus Vahrenkamp
*             GNU Lesser General Public License
*
*/
#ifndef _VirtualRobot_Visualization_h_
#define _VirtualRobot_Visualization_h_

#include "../Model/Frame.h"
#include "../Model/Primitive.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <functional>
#include <map>

namespace VirtualRobot
{
    class TriMeshModel;
    class BoundingBox;
    class VisualizationFactory;
    class VisualizationSet;

    class VIRTUAL_ROBOT_IMPORT_EXPORT Visualization : public Frame
    {
        friend class VisualizationSet;
    public:
        struct Color
        {
            Color()
            {
                transparency = 0.0f;
                r = g = b = 0.5f;
            }
            Color(float r, float g, float b, float transparency = 0.0f): r(r), g(g), b(b), transparency(transparency) {}
            float r, g, b;
            float transparency;
            bool isNone() const
            {
                return transparency >= 1.0f;
            }
            bool isTransparencyOnly()
            {
                return r > 1.f || g > 1.f || b > 1.f || r < 0.f || g < 0.f || b < 0.f;
            }
            static Color Blue(float transparency = 0.0f)
            {
                return Color(0.2f, 0.2f, 1.0f, transparency);
            }
            static Color Red(float transparency = 0.0f)
            {
                return Color(1.0f, 0.2f, 0.2f, transparency);
            }
            static Color Green(float transparency = 0.0f)
            {
                return Color(0.2f, 1.0f, 0.2f, transparency);
            }
            static Color Black(float transparency = 0.0f)
            {
                return Color(0, 0, 0, transparency);
            }
            static Color Gray()
            {
                return Color(0.5f, 0.5f, 0.5f, 0);
            }
            static Color None()
            {
                return Color(0.0f, 0.0f, 0.0f, 1.0f);
            }
            static Color Transparency(float transparency)
            {
                return Color(-1.f, -1.f, -1.f, transparency);
            }
        };

        struct PhongMaterial
        {
            PhongMaterial() {}
            Color emission;
            Color ambient;
            Color diffuse;
            Color specular;
            float shininess;
            Color reflective;
            float reflectivity;
            Color transparent;
            float transparency;
            float refractionIndex;
        };

    protected:
        /*!
        Constructor
        */
        Visualization() : Frame()
        {
        }

    public:
        /*!
        */
        virtual ~Visualization() = default;

        /*!
            Sets the position of the internal data structure.
        */
        virtual void setGlobalPose(const Eigen::Matrix4f& m);
        virtual void applyDisplacement(const Eigen::Matrix4f& dp);
        virtual size_t addPoseChangedCallback(std::function<void(const Eigen::Matrix4f&)> f) = 0;
        virtual void removePoseChangedCallback(size_t id) = 0;

        /*!
            Set the visibility of this visualisation.
            \param visible If false, the visualization is not shown.
        */
        virtual void setVisible(bool showVisualization) = 0;
        virtual bool isVisible() const = 0;

        /*!
            Enables/Disables the visualization updates.
            Usually if a model node changes its state, the visualization is automatically updated.
            This behavior can be changed here.
        */
        virtual void setUpdateVisualization(bool enable) = 0;
        virtual bool getUpdateVisualizationStatus() const = 0;


        enum DrawStyle
        {
            normal,
            wireframe
        };
        virtual void setStyle(DrawStyle s) = 0;
        virtual DrawStyle getStyle() const = 0;

        virtual void setColor(const Color& c) = 0;
        virtual Color getColor() const = 0;
        /*!
            Colorize this visualization, but just set the transparency flag (no additional colorization is performed).
            @param transparency The transparent value in [0..1].
        */
        inline void setTransparency(float transparency)
        {
            this->setColor(Color::Transparency(transparency));
        }

        inline void select()
        {
            this->setSelected(true);
        }
        inline void deselect()
        {
            this->setSelected(false);
        }
        void setSelected(bool selected)
        {
            if (this->isInVisualizationSet())
            {
                VR_WARNING << "Selection status of visualization could not be changed, because it is part of a set." << std::endl;
            }
            else
            {
                this->_setSelected(selected);
            }
        }
        virtual bool isSelected() const = 0;
        virtual size_t addSelectionChangedCallback(std::function<void(bool)> f) = 0;
        virtual void removeSelectionChangedCallback(size_t id) = 0;

        virtual void scale(const Eigen::Vector3f& scaleFactor) = 0;
        virtual Eigen::Vector3f getScaleFactor() const = 0;

        virtual void shrinkFatten(float offset) = 0;

        enum ManipulatorType
        {
            position,
            rotation
        };
        void addManipulator(ManipulatorType t)
        {
            if (this->isInVisualizationSet())
            {
                VR_WARNING << "Manipulator could not be added to visualization, because it is part of a set." << std::endl;
            }
            else
            {
                this->_addManipulator(t);
            }
        }
        void removeManipulator(ManipulatorType t)
        {
            if (this->isInVisualizationSet())
            {
                VR_WARNING << "Manipulator could not be removed from visualization, because it is part of a set." << std::endl;
            }
            else
            {
                this->_removeManipulator(t);
            }
        }
        virtual bool hasManipulator(ManipulatorType t) const = 0;
        virtual std::vector<ManipulatorType> getAddedManipulatorTypes() const = 0;
        void removeAllManipulators()
        {
            if (this->isInVisualizationSet())
            {
                VR_WARNING << "Manipulator could not be removed from visualization, because it is part of a set." << std::endl;
            }
            else
            {
                this->_removeAllManipulators();
            }
        }

        virtual std::vector<Primitive::PrimitivePtr> getPrimitives() const = 0;

        //! Just stores the filename, no loading is performed!
        virtual void setFilename(const std::string& filename, bool boundingBox) = 0;
        //! optional filename tag
        virtual std::string getFilename() const = 0;
        virtual bool usedBoundingBoxVisu() const = 0;

        /*!
            Returns (current) bounding box in global coordinate system.
        */
        virtual BoundingBox getBoundingBox() const = 0;

        /*!
            Creates a triangulated model.
        */
        virtual TriMeshModelPtr getTriMeshModel() const = 0;
        //! update trimesh model
        virtual void createTriMeshModel() = 0;

        //! get number of faces (i.e. triangles) of this object
        virtual int getNumFaces() const = 0;

        /*!
            Clone this visualization.
            \param deepCopy When true, the underlying visualization is copied, otherwise a reference to the existing visualization is passed.
            \param scaling Scale Can be set to create a scaled version of this visual data.
            Since the underlying implementation may be able to re-use the visualization data, a deep copy may not be necessary in some cases.
        */
        virtual VisualizationPtr clone(float scaling = 1.0f) const = 0;

        //! print information about this visualization object.
        virtual void print() const = 0;

        virtual std::string toXML(const std::string& basePath, int tabs) const = 0;

        /*!
            Ctreate XML string and replace filename
        */
        virtual std::string toXML(const std::string& basePath, const std::string& filename, int tabs) const = 0;

        /*!
            Saves model file to model path.
            \param modelPath The directory.
        */
        virtual bool saveModel(const std::string& modelPath, const std::string& filename) = 0;

        /**
         * @brief setIsInVisualizationSet Internally used function to determinate if this visualization is in a set.
         *
         * If a visualization is in a set, it is only selecable and manipulateable using the set.
         */
        virtual bool isInVisualizationSet() const = 0;
    protected:
        virtual void setIsInVisualizationSet(bool inSet) = 0;
        virtual void _setSelected(bool selected) = 0;
        virtual void _addManipulator(ManipulatorType t) = 0;
        virtual void _removeManipulator(ManipulatorType t) = 0;
        virtual void _removeAllManipulators() = 0;
    };

    class VIRTUAL_ROBOT_IMPORT_EXPORT DummyVisualization : virtual public Visualization
    {
        friend class VisualizationFactory;
    protected:
        /*!
        Constructor
        */
        DummyVisualization();

    public:
        /*!
        */
        virtual ~DummyVisualization() override = default;

        virtual void setGlobalPose(const Eigen::Matrix4f& m) override;
        //virtual void applyDisplacement(const Eigen::Matrix4f& dp) override;
        virtual size_t addPoseChangedCallback(std::function<void(const Eigen::Matrix4f&)> f) override;
        virtual void removePoseChangedCallback(size_t id) override;

        /*!
            Set the visibility of this visualisation.
            \param visible If false, the visualization is not shown.
        */
        virtual void setVisible(bool showVisualization) override;
        virtual bool isVisible() const override;

        /*!
            Enables/Disables the visualization updates.
            Usually if a model node changes its state, the visualization is automatically updated.
            This behavior can be changed here.
        */
        virtual void setUpdateVisualization(bool enable) override;
        virtual bool getUpdateVisualizationStatus() const override;

        virtual void setStyle(DrawStyle s) override;
        virtual DrawStyle getStyle() const override;

        virtual void setColor(const Color& c) override;
        virtual Color getColor() const override;

    protected:
        virtual void _setSelected(bool selected) override;
    public:
        virtual bool isSelected() const override;
        virtual size_t addSelectionChangedCallback(std::function<void(bool)> f) override;
        virtual void removeSelectionChangedCallback(size_t id) override;

        virtual void scale(const Eigen::Vector3f& scaleFactor) override;
        virtual Eigen::Vector3f getScaleFactor() const override;

        virtual void shrinkFatten(float offset) override;

    protected:
        virtual void _addManipulator(ManipulatorType t) override;
        virtual void _removeManipulator(ManipulatorType t) override;
        virtual void _removeAllManipulators() override;
    public:
        virtual bool hasManipulator(ManipulatorType t) const override;
        virtual std::vector<ManipulatorType> getAddedManipulatorTypes() const override;

        virtual std::vector<Primitive::PrimitivePtr> getPrimitives() const override;

        //! Just stores the filename, no loading is performed!
        virtual void setFilename(const std::string& filename, bool boundingBox) override;
        //! optional filename tag
        virtual std::string getFilename() const override;
        virtual bool usedBoundingBoxVisu() const override;

        /*!
            Returns (current) bounding box in global coordinate system.
        */
        virtual BoundingBox getBoundingBox() const override;

        /*!
            Creates a triangulated model.
        */
        virtual TriMeshModelPtr getTriMeshModel() const override;
        //! update trimesh model
        virtual void createTriMeshModel() override;

        //! get number of faces (i.e. triangles) of this object
        virtual int getNumFaces() const override;

        /*!
            Clone this visualization.
            \param deepCopy When true, the underlying visualization is copied, otherwise a reference to the existing visualization is passed.
            \param scaling Scale Can be set to create a scaled version of this visual data.
            Since the underlying implementation may be able to re-use the visualization data, a deep copy may not be necessary in some cases.
        */
        virtual VisualizationPtr clone(float scaling = 1.0f) const override;

        //! print information about this visualization object.
        virtual void print() const override;

        virtual std::string toXML(const std::string& basePath, int tabs) const override;

        /*!
            Ctreate XML string and replace filename
        */
        virtual std::string toXML(const std::string& basePath, const std::string& filename, int tabs) const override;

        /*!
            Saves model file to model path.
            \param modelPath The directory.
        */
        virtual bool saveModel(const std::string& modelPath, const std::string& filename) override;

        /**
         * @brief setIsInVisualizationSet Internally used function to determinate if this visualization is in a set.
         *
         * If a visualization is in a set, it is only selecable and manipulateable using the set.
         */
        virtual bool isInVisualizationSet() const override;
    protected:
        virtual void setIsInVisualizationSet(bool inSet);

        bool visible;
        bool updateVisualization;
        DrawStyle style;
        Color color;
        bool selected;
        std::set<ManipulatorType> addedManipulators;
        std::string filename; //!< if the visualization was build from a file, the filename is stored here
        bool boundingBox; //!< Indicates, if the bounding box model was used
        std::vector<Primitive::PrimitivePtr> primitives;
        bool inVisualizationSet;
        Eigen::Vector3f scaleFactor;
        TriMeshModelPtr triMeshModel;
        std::map<unsigned int, std::function<void(const Eigen::Matrix4f&)>> poseChangedCallbacks;
        std::map<unsigned int, std::function<void(bool)>> selectionChangedCallbacks;
    };

} // namespace VirtualRobot

#endif // _VirtualRobot_Visualization_h_
