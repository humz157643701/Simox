#include "ModelJoint.h"
#include "../../VirtualRobotException.h"

namespace VirtualRobot
{
    ModelJoint::ModelJoint(const ModelWeakPtr& model,
                           const std::string& name,
                           const Eigen::Matrix4f& staticTransformation,
                           float jointLimitLo,
                           float jointLimitHi,
                           float jointValueOffset) : ModelNode(model, name, staticTransformation),
                                                     jointValue(0),
                                                     jointValueOffset(jointValueOffset),
                                                     jointLimitLo(jointLimitLo),
                                                     jointLimitHi(jointLimitHi),
                                                     maxVelocity(-1.0f),
                                                     maxAcceleration(-1.0f),
                                                     maxTorque(-1.0f)
    {
    }

    ModelJoint::~ModelJoint()
    {
    }
	/*
    ModelNode::ModelNodeType ModelJoint::getType() const
    {
        return ModelNode::ModelNodeType::Joint;
    }*/

    void ModelJoint::setJointValue(float q)
    {
        setJointValueNoUpdate(q);
        updatePose(true, true);
    }

    void ModelJoint::setJointValueNoUpdate(float q)
    {
        VR_ASSERT_MESSAGE((!std::isnan(q) && !std::isinf(q)) , "Not a valid number...");

        respectJointLimits(q);

        WriteLockPtr w = getModel()->getWriteLock();
        jointValue = q;
    }

    float ModelJoint::getJointValue() const
    {
        ReadLockPtr r = getModel()->getReadLock();
        return jointValue;
    }

    bool ModelJoint::checkJointLimits(float jointValue, bool verbose) const
    {
        bool res = true;

        if (jointValue < getJointLimitLow())
        {
            res = false;
        }

        if (jointValue > getJointLimitHigh())
        {
            res = false;
        }

        if (!res && verbose)
        {
            VR_INFO << "Joint: " << getName() << ":"
                    << " joint value (" << jointValue << ")"
                    << " is out of joint boundaries (lo:" << jointLimitLo << ", hi: " << jointLimitHi << ")" << endl;
        }

        return res;
    }

    void ModelJoint::respectJointLimits(float& jointValue) const
    {
        ReadLockPtr r = getModel()->getReadLock();
        if (jointValue < jointLimitLo)
        {
            jointValue = jointLimitLo;
        }

        if (jointValue > jointLimitHi)
        {
            jointValue = jointLimitHi;
        }
    }

    void ModelJoint::setJointLimits(float lo, float hi)
    {
        WriteLockPtr w = getModel()->getWriteLock();
        jointLimitHi = hi;
        jointLimitLo = lo;
    }

    float ModelJoint::getJointValueOffset() const
    {
        // never updated -> no lock needed
        return jointValueOffset;
    }


    float ModelJoint::getJointLimitHigh() const
    {
        ReadLockPtr lock = getModel()->getReadLock();
        return jointLimitHi;
    }


    float ModelJoint::getJointLimitLow() const
    {
        ReadLockPtr lock = getModel()->getReadLock();
        return jointLimitLo;
    }

    void ModelJoint::setMaxVelocity(float maxVel)
    {
        WriteLockPtr w = getModel()->getWriteLock();
        maxVelocity = maxVel;
    }

    void ModelJoint::setMaxAcceleration(float maxAcc)
    {
        WriteLockPtr w = getModel()->getWriteLock();
        maxAcceleration = maxAcc;
    }

    void ModelJoint::setMaxTorque(float maxTo)
    {
        WriteLockPtr w = getModel()->getWriteLock();
        maxTorque = maxTo;
    }

    float ModelJoint::getMaxVelocity() const
    {
        ReadLockPtr r = getModel()->getReadLock();
        return maxVelocity;
    }

    float ModelJoint::getMaxAcceleration() const
    {
        ReadLockPtr r = getModel()->getReadLock();
        return maxAcceleration;
    }

    float ModelJoint::getMaxTorque() const
    {
        ReadLockPtr r = getModel()->getReadLock();
        return maxTorque;
    }

    void ModelJoint::propagateJointValue(const std::string& jointName, float factor)
    {
        WriteLockPtr w = getModel()->getWriteLock();
        if (factor == 0.0f)
        {
            propagatedJointValues.erase(jointName);
        }
        else
        {
            propagatedJointValues[jointName] = factor;
        }
    }

    void ModelJoint::updatePoseInternally(bool updateChildren, bool updateAttachments)
    {
        ModelPtr modelShared = getModel();

        for (auto it = propagatedJointValues.begin(); it != propagatedJointValues.end(); it++)
        {
            ModelNodePtr node = modelShared->getModelNode(it->first);

            if (!node || !checkNodeOfType(node, ModelNode::ModelNodeType::Joint))
            {
                VR_WARNING << "Could not propagate joint value from " << getName()
                           << " to " << it->first << " because dependent joint does not exist..." << std::endl;
            }
            else
            {
                // TODO: check, if update is needed
                std::static_pointer_cast<ModelJoint>(node)->setJointValue(jointValue * it->second);
            }
        }
        ModelNode::updatePoseInternally(updateChildren, updateAttachments);
    }
}