
#include "CDManager.h"

#include <iostream>
#include <set>
#include <float.h>
#include "../Model/Model.h"


using namespace std;

//#define CCM_DEBUG

namespace VirtualRobot
{

    CDManager::CDManager(CollisionCheckerPtr colChecker)
    {
        if (colChecker == NULL)
        {
            this->colChecker = VirtualRobot::CollisionChecker::getGlobalCollisionChecker();
        }
        else
        {
            this->colChecker = colChecker;
        }
    }

    CDManager::~CDManager()
    {
    }


    void CDManager::addCollisionModel(ModelPtr m)
    {
        VR_ASSERT(m);
        addCollisionModel(m->getLinkSet());
    }


    void CDManager::addCollisionModel(LinkSetPtr m)
    {
        if (m)
        {
            if (m->getCollisionChecker() != colChecker)
            {
                VR_WARNING << "CollisionModel is linked to different instance of collision checker..." << endl;
            }

            for (size_t i = 0; i < colModels.size(); i++)
            {
                if (m != colModels[i])
                {
                    addCollisionModelPair(colModels[i], m);
                }
            }

            if (!_hasSceneObjectSet(m))
            {
                colModels.push_back(m);
            }
        }
    }

    void CDManager::addCollisionModel(ModelLinkPtr m)
    {
        if (m)
        {
            std::vector<ModelNodePtr> models;
            models.push_back(m);
            VirtualRobot::LinkSetPtr cms = VirtualRobot::LinkSet::createLinkSet(m->getModel(), std::string(""), models);
            addCollisionModel(cms);
        }
    }

    void CDManager::addCollisionModel(const std::vector<ModelLinkPtr>& m)
    {
        if (m.size()>0)
        {
            VirtualRobot::LinkSetPtr cms = VirtualRobot::LinkSet::createLinkSet(m.at(0)->getModel(), std::string(""), m);
            addCollisionModel(cms);
        }
    }

    bool CDManager::isInCollision(LinkSetPtr m)
    {
        if (!m || !colChecker)
        {
            VR_WARNING << " NULL data..." << endl;
            return false;
        }

        // check all colmodels
        for (unsigned int i = 0; i < colModels.size(); i++)
        {
            if (m != colModels[i])
            {
                if (colChecker->checkCollision(colModels[i], m))
                {
                    return true;
                }
            }
        }

        // if here -> no collision
        return false;
    }


    float CDManager::getDistance(LinkSetPtr m)
    {
        float minDist = FLT_MAX;
        float tmp;

        if (!m || !colChecker)
        {
            VR_WARNING << " NULL data..." << endl;
            return 0.0f;
        }

        // check all colmodels
        for (unsigned int i = 0; i < colModels.size(); i++)
        {
            if (m != colModels[i])
            {
                tmp = (float)colChecker->calculateDistance(colModels[i], m);

                if (tmp < minDist)
                {
                    minDist = tmp;
                }
            }
        }

        return minDist;
    }



    float CDManager::getDistance(LinkSetPtr m, std::vector<LinkSetPtr>& sets)
    {
        float minDist = FLT_MAX;

        for (size_t i = 0; i < sets.size(); i++)
        {
            float tmp = (float)colChecker->calculateDistance(m, sets[i]);

            if (tmp < minDist)
            {
                minDist = tmp;
            }
        }

        return minDist;
    }


    float CDManager::getDistance()
    {
        float minDist = FLT_MAX;
        float tmp;

        if (!colChecker)
        {
            return -1.0f;
        }

        std::map< LinkSetPtr, std::vector<LinkSetPtr> >::iterator i = colModelPairs.begin();

        while (i != colModelPairs.end())
        {
            tmp = getDistance(i->first, i->second);

            if (tmp < minDist)
            {
                minDist = tmp;
            }

            i++;
        }

        return minDist;
    }

    float CDManager::getDistance(LinkSetPtr m, std::vector<LinkSetPtr>& sets, Eigen::Vector3f& P1, Eigen::Vector3f& P2, int& trID1, int& trID2)
    {
        float minDist = FLT_MAX;
        Eigen::Vector3f _P1;
        Eigen::Vector3f _P2;
        int _trID1;
        int _trID2;

        for (size_t i = 0; i < sets.size(); i++)
        {
            float tmp = (float)colChecker->calculateDistance(m, sets[i], _P1, _P2, &_trID1, &_trID2);

            if (tmp < minDist)
            {
                minDist = tmp;
                trID1 = _trID1;
                P1[0] = _P1[0];
                P1[1] = _P1[1];
                P1[2] = _P1[2];
                trID2 = _trID2;
                P2[0] = _P2[0];
                P2[1] = _P2[1];
                P2[2] = _P2[2];
            }
        }

        return minDist;
    }

    float CDManager::getDistance(Eigen::Vector3f& P1, Eigen::Vector3f& P2, int& trID1, int& trID2)
    {
        float minDist = FLT_MAX;
        float tmp;
        Eigen::Vector3f _P1;
        Eigen::Vector3f _P2;
        int _trID1;
        int _trID2;

        if (!colChecker)
        {
            return -1.0f;
        }


        std::map< LinkSetPtr, std::vector<LinkSetPtr> >::iterator i = colModelPairs.begin();

        while (i != colModelPairs.end())
        {
            tmp = getDistance(i->first, i->second, _P1, _P2, _trID1, _trID2);

            if (tmp < minDist)
            {
                minDist = tmp;
                trID1 = _trID1;
                P1[0] = _P1[0];
                P1[1] = _P1[1];
                P1[2] = _P1[2];
                trID2 = _trID2;
                P2[0] = _P2[0];
                P2[1] = _P2[1];
                P2[2] = _P2[2];
            }

            i++;
        }

        return minDist;
    }

    float CDManager::getDistance(LinkSetPtr m, Eigen::Vector3f& P1, Eigen::Vector3f& P2, int& trID1, int& trID2)
    {
        float minDist = FLT_MAX;
        float tmp;
        Eigen::Vector3f _P1;
        Eigen::Vector3f _P2;
        int _trID1;
        int _trID2;

        if (!colChecker || !m)
        {
            return -1.0f;
        }

        for (unsigned int j = 0; j < colModels.size(); j++)
        {
            tmp = (float)colChecker->calculateDistance(m, colModels[j], _P1, _P2, &_trID1, &_trID2);

            if (tmp < minDist)
            {
                minDist = tmp;
                trID1 = _trID1;
                P1[0] = _P1[0];
                P1[1] = _P1[1];
                P1[2] = _P1[2];
                trID2 = _trID2;
                P2[0] = _P2[0];
                P2[1] = _P2[1];
                P2[2] = _P2[2];
            }
        }

        return minDist;
    }



    bool CDManager::isInCollision(LinkSetPtr m, std::vector<LinkSetPtr>& sets)
    {
        for (size_t i = 0; i < sets.size(); i++)
        {
            if (colChecker->checkCollision(m, sets[i]))
            {
                return true;
            }
        }

        return false;
    }

    bool CDManager::isInCollision()
    {
        if (!colChecker)
        {
            return false;
        }

        std::map<LinkSetPtr, std::vector<LinkSetPtr>  >::iterator i = colModelPairs.begin();

        while (i != colModelPairs.end())
        {
            if (isInCollision(i->first, i->second))
            {
                return true;
            }

            i++;
        }

        return false;
    }


    std::vector<LinkSetPtr> CDManager::getSceneObjectSets()
    {
        return colModels;
    }

    CollisionCheckerPtr CDManager::getCollisionChecker()
    {
        return colChecker;
    }

    bool CDManager::hasSceneObjectSet(LinkSetPtr m)
    {
        for (size_t i = 0; i < colModels.size(); i++)
        {
            if (colModels[i] == m)
            {
                return true;
            }
        }

        return false;
    }

    bool CDManager::_hasSceneObjectSet(LinkSetPtr m)
    {
        for (size_t i = 0; i < colModels.size(); i++)
        {
            if (colModels[i] == m)
            {
                return true;
            }

            if (m->getSize() == 1 && colModels[i]->getSize() == 1 &&  colModels[i]->getNode(0) == m->getNode(0))
            {
                return true;
            }
        }

        return false;
    }

    bool CDManager::hasSceneObject(ModelLinkPtr m)
    {
        for (size_t i = 0; i < colModels.size(); i++)
        {
            if (colModels[i]->getSize() == 1 &&  colModels[i]->getNode(0) == m)
            {
                return true;
            }
        }

        return false;
    }

    void CDManager::addCollisionModelPair(LinkSetPtr m1, LinkSetPtr m2)
    {
        if (!m1 || !m2)
        {
            return;
        }

        if (!_hasSceneObjectSet(m1))
        {
            colModels.push_back(m1);
        }

        if (!_hasSceneObjectSet(m2))
        {
            colModels.push_back(m2);
        }

        colModelPairs[m1].push_back(m2);
    }

    void CDManager::addCollisionModelPair(ModelLinkPtr m1, LinkSetPtr m2)
    {
        if (!m1 || !m2)
        {
            return;
        }

        std::vector<ModelNodePtr> models;
        models.push_back(m1);
        VirtualRobot::LinkSetPtr cms = VirtualRobot::LinkSet::createLinkSet(m1->getModel(), "", models);
        addCollisionModelPair(cms, m2);
    }

    void CDManager::addCollisionModelPair(ModelLinkPtr m1, ModelLinkPtr m2)
    {
        if (!m1 || !m2)
        {
            return;
        }

        std::vector<ModelNodePtr> models1;
        models1.push_back(m1);
        VirtualRobot::LinkSetPtr cms = VirtualRobot::LinkSet::createLinkSet(m1->getModel(), "", models1);
        std::vector<ModelNodePtr> models2;
        models2.push_back(m2);
        VirtualRobot::LinkSetPtr cms2 = VirtualRobot::LinkSet::createLinkSet(m2->getModel(), "", models2);
        addCollisionModelPair(cms, cms2);
    }
}
