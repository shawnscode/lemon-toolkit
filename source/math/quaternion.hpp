// @date 2016/08/15
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/matrix.hpp>

NS_LEMON_MATH_BEGIN

// a quaternion is given by q = x*i + y*j + z*k + w, where x, y, z and w are real numbers.
struct Quaternion
{
    using value_type = float;

    Quaternion() : _tuple({value_type(1), value_type(0), value_type(0), value_type(0)}) {}
    Quaternion(value_type w, value_type x, value_type y, value_type z) : _tuple({w, x, y, z}) {}

    Quaternion(const Quaternion&) = default;
    Quaternion& operator = (const Quaternion&) = default;

    const value_type& operator[](size_t) const;
    value_type& operator[](size_t);

protected:
    Vector4<value_type> _tuple;
};

using Vector3T = Vector3<Quaternion::value_type>;
using Vector4T = Vector4<Quaternion::value_type>;
using Matrix3T = Matrix3<Quaternion::value_type>;
using Matrix4T = Matrix4<Quaternion::value_type>;
using quat_value_type = Quaternion::value_type;

// test for (in)equality with another quaternion without epsilon
bool operator == (const Quaternion&, const Quaternion&);
bool operator != (const Quaternion&, const Quaternion&);

// basic algebraic operations
Quaternion operator + (const Quaternion&);
Quaternion operator - (const Quaternion&);

Quaternion operator + (const Quaternion&, const Quaternion&);
Quaternion operator - (const Quaternion&, const Quaternion&);
Quaternion operator * (const Quaternion&, const Quaternion&);
Quaternion operator / (const Quaternion&, const Quaternion&);
Quaternion operator * (const Quaternion&, quat_value_type);

Quaternion& operator += (Quaternion&, const Quaternion&);
Quaternion& operator -= (Quaternion&, const Quaternion&);
Quaternion& operator *= (Quaternion&, const Quaternion&);
Quaternion& operator /= (Quaternion&, const Quaternion&);
Quaternion& operator *= (Quaternion&, quat_value_type);

Vector3T   operator * (const Vector3T&, const Quaternion&);
Vector3T   operator / (const Vector3T&, const Quaternion&);

// comparison with epslion
bool equals(const Quaternion&, const Quaternion&, quat_value_type e = epsilon<quat_value_type>());

// build a quaternion from euler angles(pitch, yaw, roll), in degree
// order of rotations: Z first, then X, then Y
Quaternion from_euler_angles(const Vector3T&);
// build a quaternion from an angle(in degree) and axis
Quaternion from_axis_angle(quat_value_type, const Vector3T&);
// build a quaternion from the rotation difference between two direction vectors.
Quaternion from_rotation_to(const Vector3T&, const Vector3T&);
// build a quaternion from rotation matrix
Quaternion from_rotation_matrix(const Matrix3T&);
Quaternion from_rotation_matrix(const Matrix4T&);

// return squared length
quat_value_type length_square(const Quaternion&);
// calculate dot product
quat_value_type dot(const Quaternion&, const Quaternion&);
// return conjugate
Quaternion conjugate(const Quaternion&);
// return normalized quaternion
Quaternion normalize(const Quaternion&);
// return inverse
Quaternion inverse(const Quaternion&);

// return the quaternion rotation angle in degree
quat_value_type angle(const Quaternion&);
// return euler angles in degree
quat_value_type pitch(const Quaternion&);
quat_value_type roll(const Quaternion&);
quat_value_type yaw(const Quaternion&);
// order of rotations: Z first, then X, then Y
Vector3T to_euler_angles(const Quaternion&);
// return the rotation matrix that correspons to this quaternion
Matrix3T to_rotation_matrix(const Quaternion&);
// normalized linear interpolation with another quaternion
Quaternion lerp(const Quaternion&, const Quaternion&, float, bool shortest = false);
// spherical interpolation with another quaternion
Quaternion slerp(const Quaternion&, const Quaternion&, float);

#include <math/quaternion.inl>
NS_LEMON_MATH_END