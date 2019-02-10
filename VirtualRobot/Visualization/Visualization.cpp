/**
* @package    VirtualRobot
* @author     Manfred Kroehnert
* @author     Nikolaus Vahrenkamp
* #author     Adrian Knobloch
* @copyright  2010 Manfred Kroehnert
*/


#include "Visualization.h"
#include "SelectionGroup.h"
#include "SelectionManager.h"
#include "TriMeshModel.h"
#include "../Tools/BoundingBox.h"

#include "VirtualRobot/Model/Model.h"
#include "VirtualRobot/VirtualRobotException.h"
#include "VirtualRobot/XML/BaseIO.h"


namespace VirtualRobot
{

    DummyVisualization::DummyVisualization() :
        Visualization(),
        visible(true),
        updateVisualization(true),
        style(Visualization::DrawStyle::normal),
        material(new NoneMaterial),
        filename(""),
        boundingBox(false),
        primitives()
    {
    }

    void DummyVisualization::init()
    {
        Visualization::init();
        createTriMeshModel();
    }

    Visualization::Visualization() : Frame()
    {
    }

    void VirtualRobot::Visualization::init()
    {
        selectionGroup = VisualizationFactory::getInstance()->createSelectionGroup();
        selectionGroup->addVisualization(shared_from_this());

        setGlobalPose(Eigen::Matrix4f::Identity());
        setVisible(true);
        setUpdateVisualization(true);
        setStyle(DrawStyle::normal);
        setColor(Color::None());
        setSelected(false);
    }

    void Visualization::setGlobalPose(const Eigen::Matrix4f &m)
    {
        if (m != getGlobalPose())
        {
            for (auto& f : poseChangedCallbacks)
            {
                f.second(m);
            }
        }
        Frame::setGlobalPose(m);
    }

    void Visualization::applyDisplacement(const Eigen::Matrix4f &dp)
    {
        this->setGlobalPose(this->getGlobalPose() * dp);
    }

    size_t Visualization::addPoseChangedCallback(std::function<void (const Eigen::Matrix4f &)> f)
    {
        static size_t id = 0;
        poseChangedCallbacks[id] = f;
        return id++;
    }

    void Visualization::removePoseChangedCallback(size_t id)
    {
        auto it = poseChangedCallbacks.find(id);
        if (it != poseChangedCallbacks.end())
        {
            poseChangedCallbacks.erase(it);
        }
    }

    void Visualization::setSelected(bool selected)
    {
        getSelectionGroup()->setSelected(selected);
    }

    bool Visualization::isSelected() const
    {
        return getSelectionGroup()->isSelected();
    }

    size_t Visualization::addSelectionChangedCallback(std::function<void (bool)> f)
    {
        static unsigned int id = 0;
        selectionChangedCallbacks[id] = f;
        return id++;
    }

    void Visualization::removeSelectionChangedCallback(size_t id)
    {
        auto it = selectionChangedCallbacks.find(id);
        if (it != selectionChangedCallbacks.end())
        {
            selectionChangedCallbacks.erase(it);
        }
    }

    void Visualization::executeSelectionChangedCallbacks(bool selected)
    {
        for (auto& f : selectionChangedCallbacks)
        {
            f.second(selected);
        }
    }

    void Visualization::setSelectionGroup(const SelectionGroupPtr &group)
    {
        VR_ASSERT(selectionGroup);
        bool selected = selectionGroup->isSelected();
        auto oldGroup = selectionGroup;
        selectionGroup->removeVisualization(shared_from_this());
        selectionGroup = group ? group : VisualizationFactory::getInstance()->createSelectionGroup();
        selectionGroup->addVisualization(shared_from_this());
        SelectionManager::getInstance()->emitSlectionGroupChanged(shared_from_this(), oldGroup, selectionGroup);
        if (selectionGroup->isSelected() != selected)
        {
            executeSelectionChangedCallbacks(selectionGroup->isSelected());
        }
    }

    SelectionGroupPtr Visualization::getSelectionGroup() const
    {
        VR_ASSERT(selectionGroup); // A Visualization must have a selection group, but a VisualizationSet not!
        return selectionGroup;
    }

    void Visualization::addMutex(const std::shared_ptr<std::recursive_mutex> &m)
    {
        std::lock_guard<std::mutex> l(mutexListChangeMutex);
        mutexList.push_back(m);
        VR_ASSERT(m);
        std::sort(mutexList.begin(), mutexList.end());
    }

    void Visualization::removeMutex(const std::shared_ptr<std::recursive_mutex> &m)
    {
        std::lock_guard<std::mutex> l(mutexListChangeMutex);
        mutexList.erase(std::remove(mutexList.begin(), mutexList.end(), m), mutexList.end());
    }

    void Visualization::swapMutex(const std::shared_ptr<std::recursive_mutex> &oldM, const std::shared_ptr<std::recursive_mutex> &newM)
    {
        std::lock_guard<std::mutex> l(mutexListChangeMutex);
        VR_ASSERT(newM);
        auto it = std::find(mutexList.begin(), mutexList.end(), oldM);
        if (it == mutexList.end())
        {
            VR_ERROR << "Old mutex not found";
            mutexList.push_back(newM);
        }
        else
        {
            *it = newM;
        }
        std::sort(mutexList.begin(), mutexList.end());
    }

    std::shared_ptr<MultipleMutexLockGuard> Visualization::getScopedLock() const
    {
        return std::shared_ptr<MultipleMutexLockGuard>(new MultipleMutexLockGuard(mutexList));
    }

    void DummyVisualization::setVisible(bool showVisualization)
    {
        visible = showVisualization;
    }

    bool DummyVisualization::isVisible() const
    {
        return visible;
    }

    void DummyVisualization::setUpdateVisualization(bool enable)
    {
        updateVisualization = enable;
    }

    bool DummyVisualization::getUpdateVisualizationStatus() const
    {
        return updateVisualization;
    }

    void DummyVisualization::setStyle(Visualization::DrawStyle s)
    {
        style = s;
    }

    Visualization::DrawStyle DummyVisualization::getStyle() const
    {
        return style;
    }

    void DummyVisualization::setColor(const Visualization::Color &c)
    {
        if (!c.isNone())
        {
            PhongMaterialPtr m = std::dynamic_pointer_cast<PhongMaterial>(material);
            if (!m)
            {
                m.reset(new PhongMaterial);
            }
            m->diffuse = c;
            m->ambient = c;
            setMaterial(m);
        }
    }

    Visualization::Color DummyVisualization::getColor() const
    {
        PhongMaterialPtr m = std::dynamic_pointer_cast<PhongMaterial>(material);
        if (!m)
        {
            return Color::None();
        }
        return m->diffuse;
    }

    void DummyVisualization::setMaterial(const Visualization::MaterialPtr &material)
    {
        this->material = material;
    }

    Visualization::MaterialPtr DummyVisualization::getMaterial() const
    {
        return material;
    }

    void DummyVisualization::scale(const Eigen::Vector3f &)
    {
        VR_ERROR_ONCE_NYI;
    }

    void DummyVisualization::shrinkFatten(float offset)
    {
        this->createTriMeshModel();
        this->getTriMeshModel()->mergeVertices();
        this->getTriMeshModel()->fattenShrink(offset);
    }

    std::vector<Primitive::PrimitivePtr> DummyVisualization::getPrimitives() const
    {
        return primitives;
    }

    void DummyVisualization::setFilename(const std::string &filename, bool boundingBox)
    {
        this->filename = filename;
        this->boundingBox = boundingBox;
    }

    std::string DummyVisualization::getFilename() const
    {
        return filename;
    }

    bool DummyVisualization::usedBoundingBoxVisu() const
    {
        return boundingBox;
    }

    void DummyVisualization::getTextureFiles(std::vector<std::string> &) const
    {
    }

    BoundingBox DummyVisualization::getBoundingBox() const
    {
        TriMeshModelPtr tm = this->getTriMeshModel();
        VR_ASSERT(tm);

        VirtualRobot::BoundingBox bbox = tm->boundingBox;
        bbox.transform(this->getGlobalPose());
        return bbox;
    }

    TriMeshModelPtr DummyVisualization::getTriMeshModel() const
    {
        VR_ASSERT(triMeshModel);
        return triMeshModel;
    }

    void DummyVisualization::createTriMeshModel()
    {
        triMeshModel.reset(new TriMeshModel);
    }

    int DummyVisualization::getNumFaces() const
    {
        TriMeshModelPtr p = getTriMeshModel();
        VR_ASSERT(p);

        return static_cast<int>(p->faces.size());
    }

    VisualizationPtr DummyVisualization::clone() const
    {
        VisualizationPtr visu(new DummyVisualization);
        visu->init();

        visu->setVisible(this->isVisible());
        visu->setUpdateVisualization(this->getUpdateVisualizationStatus());
        visu->setStyle(this->getStyle());
        visu->setColor(this->getColor());
        visu->setFilename(this->getFilename(), this->usedBoundingBoxVisu());

        return VisualizationPtr();
    }

    void DummyVisualization::print() const
    {
        std::cout << "Dummy VisualizationNode" << std::endl;
    }

    std::string DummyVisualization::toXML(const std::string &basePath, int tabs) const
    {
        std::stringstream ss;
        std::string t = "\t";
        std::string pre = "";

        for (int i = 0; i < tabs; i++)
        {
            pre += "\t";
        }

        ss << pre << "<Visualization";

        if (usedBoundingBoxVisu())
        {
            ss << " BoundingBox='true'";
        }

        ss << ">\n";

        if (!filename.empty())
        {
            std::string tmpFilename = filename;
            BaseIO::makeRelativePath(basePath, tmpFilename);
            ss << pre << t << "<File type='" << VisualizationFactory::getInstance()->getVisualizationType() << "'>" << tmpFilename << "</File>\n";
        }
        else if (primitives.size() != 0)
        {
            ss << pre << "\t<Primitives>\n";
            std::vector<Primitive::PrimitivePtr>::const_iterator it;

            for (it = primitives.begin(); it != primitives.end(); it++)
            {
                ss << (*it)->toXMLString(tabs + 1);
            }

            ss << pre << "\t</Primitives>\n";
        }

        ss << pre << "</Visualization>\n";

        return ss.str();
    }

    std::string DummyVisualization::toXML(const std::string &basePath, const std::string &filename, int tabs) const
    {
        std::string visualizationFilename = getFilename();
        boost::filesystem::path fn(visualizationFilename);
        return toXML(basePath, fn.string(), tabs);
    }

    bool DummyVisualization::saveModel(const std::string &modelPath, const std::string &filename)
    {
        static bool printed = false;
        if (!printed)
        {
            VR_ERROR << __FILE__ << " " << __LINE__ << ": NYI" << std::endl;
            printed = true;
        }
        return false;
    }

    MultipleMutexLockGuard::MultipleMutexLockGuard(const std::vector<std::shared_ptr<std::recursive_mutex> > &mutexList)
        : mutexList(mutexList)
    {
        for (const auto& m : mutexList)
        {
            VR_ASSERT(m);
            m->lock();
        }
    }

    MultipleMutexLockGuard::~MultipleMutexLockGuard()
    {
        for (const auto& m : mutexList)
        {
            VR_ASSERT(m);
            m->unlock();
        }
    }

} // namespace VirtualRobot
