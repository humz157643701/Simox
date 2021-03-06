/**
* @package    VirtualRobot
* @author     Rainer Kartmann
* @copyright  2019 Rainer Kartmann
*/

#define BOOST_TEST_MODULE VirtualRobot_MathHelpersTest

#include <VirtualRobot/VirtualRobotTest.h>
#include <VirtualRobot/math/Helpers.h>
#include <string>
#include <stdio.h>
#include <random>


using namespace Eigen;
using Helpers = math::Helpers;


BOOST_AUTO_TEST_SUITE(MathHelpers)


BOOST_AUTO_TEST_CASE(test_CwiseMin_CwiseMax)
{
    Eigen::Vector3f a (-1, 3, 5), b(0, 3, 1);
    Eigen::Vector3f min (-1, 3, 1);
    Eigen::Vector3f max (0, 3, 5);
    BOOST_CHECK_EQUAL(Helpers::CwiseMin(a, b), min);
    BOOST_CHECK_EQUAL(Helpers::CwiseMax(a, b), max);
}

BOOST_AUTO_TEST_CASE(test_CwiseDivide)
{
    Eigen::Vector3f a (0, 5, -9), b(10, 2, 3);
    Eigen::Vector3f quot (0, 2.5, -3);
    BOOST_CHECK_EQUAL(Helpers::CwiseDivide(a, b), quot);
}

BOOST_AUTO_TEST_CASE(test_Swap)
{
    float a = 5, b = -10;
    Helpers::Swap(a, b);
    BOOST_CHECK_EQUAL(a, -10);
    BOOST_CHECK_EQUAL(b, 5);
}

BOOST_AUTO_TEST_CASE(test_GetRotationMatrix)
{
    Eigen::Vector3f source(1, 2, 3), target(-3, 2, 5);  // not normalized
    Eigen::Matrix3f matrix = Helpers::GetRotationMatrix(source, target);
    
    BOOST_CHECK((matrix * matrix.transpose()).isIdentity(1e-6f));
    BOOST_CHECK((matrix * source.normalized()).isApprox(target.normalized(), 1e-6f));
}

BOOST_AUTO_TEST_CASE(test_TransformPosition)
{
    Eigen::Vector3f vector(1, 2, 3);

    Eigen::Vector3f translation(4, 5, 6);
    Eigen::AngleAxisf rotation(static_cast<float>(M_PI_2), Eigen::Vector3f::UnitY());
    
    Eigen::Matrix4f transform = transform.Identity();
    
    // identity
    transform.setIdentity();
    BOOST_CHECK_EQUAL(Helpers::TransformPosition(transform, vector), 
                      vector);
    
    // translation only
    transform.setIdentity();
    Helpers::Position(transform) = translation;
    BOOST_CHECK_EQUAL(Helpers::TransformPosition(transform, vector), 
                      vector + translation);
    
    // rotation only
    transform.setIdentity();
    Helpers::Orientation(transform) = rotation.toRotationMatrix();
    BOOST_CHECK_EQUAL(Helpers::TransformPosition(transform, vector), 
                      rotation * vector);

    // full transform
    transform.setIdentity();
    Helpers::Position(transform) = translation;
    Helpers::Orientation(transform) = rotation.toRotationMatrix();
    BOOST_CHECK_EQUAL(Helpers::TransformPosition(transform, vector), 
                      rotation * vector + translation);
}


BOOST_AUTO_TEST_CASE(test_InvertPose)
{
    Eigen::Vector3f translation(4, 5, 6);
    Eigen::AngleAxisf rotation(static_cast<float>(M_PI_2), Eigen::Vector3f::UnitY());
    
    Eigen::Matrix4f pose = Helpers::Pose(translation, rotation);
    Eigen::Matrix4f inv;
    
    // in-place
    inv = pose;
    Helpers::InvertPose(inv);
    BOOST_CHECK((pose * inv).isIdentity(1e-6f));
    BOOST_CHECK((inv * pose).isIdentity(1e-6f));
    
    // returned
    inv.setIdentity();
    inv = Helpers::InvertedPose(pose);
    BOOST_CHECK((pose * inv).isIdentity(1e-6f));
    BOOST_CHECK((inv * pose).isIdentity(1e-6f));
}


BOOST_AUTO_TEST_SUITE_END()



struct BlockFixture
{
    BlockFixture()
    {
        quat = Quaternionf{
            AngleAxisf(static_cast<float>(M_PI), Vector3f::UnitZ()) 
            * AngleAxisf(static_cast<float>(M_PI_2), Vector3f::UnitY())
        };
        
        quat2 = AngleAxisf(static_cast<float>(M_PI_4), Vector3f::UnitX()) * quat;
        
        pos = Vector3f{ 1, 2, 3 };
        pos2 = Vector3f{ 4, 5, 6 };
        
        ori = quat.toRotationMatrix();
        ori2 = quat2.toRotationMatrix();
        
        pose.setIdentity();
        pose.block<3, 1>(0, 3) = pos;
        pose.block<3, 3>(0, 0) = ori;
    }
    
    Matrix4f pose;
    Vector3f pos, pos2;
    Matrix3f ori, ori2;
    Quaternionf quat, quat2;
};


BOOST_FIXTURE_TEST_SUITE(MathHelpers, BlockFixture)

using namespace math;


BOOST_AUTO_TEST_CASE(test_posBlock_const)
{
    BOOST_CHECK_EQUAL(Helpers::Position(const_cast<const Matrix4f&>(pose)), pos);
}

BOOST_AUTO_TEST_CASE(test_posBlock_nonconst)
{
    BOOST_CHECK_EQUAL(Helpers::Position(pose), pos);
    
    Helpers::Position(pose) = pos2;
    BOOST_CHECK_EQUAL(Helpers::Position(pose), pos2);
}


BOOST_AUTO_TEST_CASE(test_oriBlock_const)
{
    BOOST_CHECK_EQUAL(Helpers::Orientation(const_cast<const Eigen::Matrix4f&>(pose)), ori);
}

BOOST_AUTO_TEST_CASE(test_oriBlock_nonconst)
{
    BOOST_CHECK_EQUAL(Helpers::Orientation(pose), ori);
    
    Helpers::Orientation(pose) = ori2;
    BOOST_CHECK_EQUAL(Helpers::Orientation(pose), ori2);
}


BOOST_AUTO_TEST_CASE(test_toPose_matrix_and_quaternion)
{
    BOOST_CHECK_EQUAL(Helpers::Pose(pos, quat), pose);
}

BOOST_AUTO_TEST_CASE(test_toPose_matrix_and_rotation_matrix)
{
    BOOST_CHECK_EQUAL(Helpers::Pose(pos, ori), pose);
}


BOOST_AUTO_TEST_SUITE_END()


struct OrthogonolizeFixture
{
    void test(double angle, const Vector3d& axis, float noiseAmpl, float precAngularDist);
    Eigen::Matrix3f test(Matrix3f matrix, float noiseAmpl);
    
    template <typename Distribution>
    static Eigen::Matrix3f Random(Distribution& distrib)
    {
        static std::default_random_engine gen (42);
        return Eigen::Matrix3f::NullaryExpr([&](int) { return distrib(gen); });
    }
};


void OrthogonolizeFixture::test(
        double angle, const Vector3d& axis, float noiseAmpl, float precAngularDist)
{
    // construct matrix with double to avoid rounding errors
    Eigen::AngleAxisd rot(angle, axis);
    Eigen::Quaterniond quat(rot);
    
    Eigen::Matrix3f matrix = quat.toRotationMatrix().cast<float>();
    
    Eigen::Matrix3f orth = test(matrix, noiseAmpl);
    
    Quaternionf quatOrth(orth);
    BOOST_TEST_MESSAGE("Angular distance: " << quatOrth.angularDistance(quat.cast<float>()));
    BOOST_CHECK_LE(quatOrth.angularDistance(quat.cast<float>()), precAngularDist);
}

Matrix3f OrthogonolizeFixture::test(Matrix3f matrix, float noiseAmpl)
{
    const float PREC_ORTHOGONAL = 1e-6f;
    
    const Eigen::Vector3f pos(3, -1, 2);
    Eigen::Matrix4f pose = Helpers::Pose(pos, matrix);
    pose.row(3) << 1, 2, 3, 4;  // destroy last row
    
    BOOST_TEST_MESSAGE("Rotation matrix: \n" << matrix);
    BOOST_CHECK(math::Helpers::IsMatrixOrthogonal(matrix, PREC_ORTHOGONAL));
    
    BOOST_TEST_MESSAGE("Pose matrix: \n" << pose);
    
    
    // add noise (random coeffs are in [-1, 1])
    std::normal_distribution<float> distrib(0, noiseAmpl);
    const Eigen::Matrix3f noise = noiseAmpl * this->Random(distrib);
    
    matrix += noise;
    Helpers::Orientation(pose) += noise;
    
    BOOST_TEST_MESSAGE("Rotation matrix with noise: \n" << matrix);
    if (noiseAmpl > 0)
    {
        BOOST_CHECK(!math::Helpers::IsMatrixOrthogonal(matrix, PREC_ORTHOGONAL));
        BOOST_CHECK(!math::Helpers::IsMatrixOrthogonal(Helpers::Orientation(pose), PREC_ORTHOGONAL));
    }
    
    Eigen::Matrix3f orth = math::Helpers::Orthogonalize(matrix);
    Eigen::Matrix4f poseOrth = math::Helpers::Orthogonalize(pose);
    
    BOOST_TEST_MESSAGE("Orthogonalized: \n" << orth);
    BOOST_CHECK(math::Helpers::IsMatrixOrthogonal(orth, PREC_ORTHOGONAL));
    BOOST_TEST_MESSAGE("Q * Q.T: (should be Identitiy) \n" << (orth * orth.transpose()));

    BOOST_TEST_MESSAGE("Orthogonalized pose: \n" << poseOrth);
    BOOST_CHECK(math::Helpers::IsMatrixOrthogonal(Helpers::Orientation(poseOrth), PREC_ORTHOGONAL));
    BOOST_CHECK_EQUAL(math::Helpers::Position(poseOrth), pos);
    BOOST_CHECK_EQUAL(poseOrth.row(3).head<3>(), Eigen::Vector3f::Zero().transpose());
    BOOST_CHECK_EQUAL(poseOrth(3, 3), 1);
    
    return orth;
}


BOOST_FIXTURE_TEST_SUITE(Orthogonolization, OrthogonolizeFixture)

BOOST_AUTO_TEST_CASE(test_orthogonalize_zero_rotation)
{
    test(Eigen::Matrix3f::Identity(), 0);
    test(Eigen::Matrix3f::Identity(), 0.1f);
    
    test(0, Eigen::Vector3d::UnitX(), 0.0f, 0.0f);
    test(0, Eigen::Vector3d::UnitX(), 1e-3f, 1e-3f);
}

BOOST_AUTO_TEST_CASE(test_orthogonalize_aligned_axis)
{
    test(M_PI / 2, Eigen::Vector3d::UnitX(), 1e-3f, 1e-3f);
    test(M_PI / 2, Eigen::Vector3d::UnitX(), 0.1f, 0.2f);
    
    test(.75 * M_PI, Eigen::Vector3d::UnitZ(), 1e-3f, 1e-3f);
    test(.75 * M_PI, Eigen::Vector3d::UnitZ(), 0.1f, 0.2f);
    
    test(M_PI, Eigen::Vector3d::UnitY(), 1e-3f, 1e-3f);
    test(M_PI, Eigen::Vector3d::UnitY(), 0.1f, 0.2f);
}

BOOST_AUTO_TEST_CASE(test_orthogonalize_arbitrary_rotation)
{
    test(2.3, Eigen::Vector3d( 0.3, 1., -.5).normalized(), 1e-3f, 1e-3f);
    test(2.3, Eigen::Vector3d( 0.3, 1., -.5).normalized(), 0.1f, 0.2f);
    
    test(1.02, Eigen::Vector3d( -2, .3, -.25).normalized(), 1e-3f, 1e-3f);
    test(1.02, Eigen::Vector3d( -3,  2, -10).normalized(), 0.1f, 0.2f);
}


BOOST_AUTO_TEST_SUITE_END()

