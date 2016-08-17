// @date 2016/08/15
// @author Mao Jingkai(oammix@gmail.com)

#include <math/quaternion.hpp>

NS_FLOW2D_MATH_BEGIN

Quaternion from_euler_angles(const Vector3T& euler)
{
    Vector3T radians;
    radians[0] = to_radians(euler[0])*quat_value_type(0.5);
    radians[1] = to_radians(euler[1])*quat_value_type(0.5);
    radians[2] = to_radians(euler[2])*quat_value_type(0.5);

    quat_value_type sinx = std::sin(radians[0]);
    quat_value_type cosx = std::cos(radians[0]);
    quat_value_type siny = std::sin(radians[1]);
    quat_value_type cosy = std::cos(radians[1]);
    quat_value_type sinz = std::sin(radians[2]);
    quat_value_type cosz = std::cos(radians[2]);

    return Quaternion(
        cosy * cosx * cosz + siny * sinx * sinz,
        cosy * sinx * cosz + siny * cosx * sinz,
        siny * cosx * cosz - cosy * sinx * sinz,
        cosy * cosx * sinz - siny * sinx * cosz);
}

Quaternion from_axis_angle(quat_value_type degree, const Vector3T& axis)
{
    Vector3T naxis = normalize(axis);
    quat_value_type radians = to_radians(degree) * quat_value_type(0.5);

    quat_value_type sinr = std::sin(radians);
    quat_value_type cosr = std::cos(radians);

    return Quaternion(cosr, naxis[0] * sinr, naxis[1] * sinr, naxis[2] * sinr);
}

Quaternion from_rotation_to(const Vector3T& src, const Vector3T& dst)
{
    Vector3T nsrc = normalize(src);
    Vector3T ndst = normalize(dst);

    quat_value_type ctheta = dot(nsrc, ndst);

    if( ctheta >= ((quat_value_type)1 - epsilon<quat_value_type>()) )
        return Quaternion();

    // special case when vectors in opposite direction
    if( ctheta < (-(quat_value_type)1 + epsilon<quat_value_type>()) )
    {
        Vector3T axis = cross({(quat_value_type)0, (quat_value_type)0, (quat_value_type)1}, nsrc);
        if( length_square(axis) < epsilon<quat_value_type>() )
            axis = cross({(quat_value_type)1, (quat_value_type)0, (quat_value_type)0}, nsrc);

        axis = normalize(axis);
        return from_axis_angle((quat_value_type)pi, axis);
    }

    Vector3T axis = cross(nsrc, ndst);
    quat_value_type s = std::sqrt( ((quat_value_type)1 + ctheta) * (quat_value_type)2 );
    quat_value_type invs = (quat_value_type)1 / s;

    return Quaternion(
        s * (quat_value_type)0.5,
        axis[1] * invs,
        axis[2] * invs,
        axis[3] * invs);
}

Quaternion from_rotation_matrix(const Matrix3T& matrix)
{
    quat_value_type t = matrix[0][0] + matrix[1][1] + matrix[2][2];
    if( t > (quat_value_type)0 )
    {
        quat_value_type invs = (quat_value_type)0.5 / std::sqrt((quat_value_type)1.0 + t);
        return Quaternion(
            (quat_value_type)0.25 / invs,
            (matrix[2][1] - matrix[1][2]) * invs,
            (matrix[0][2] - matrix[2][0]) * invs,
            (matrix[1][0] - matrix[0][1]) * invs);
    }
    else
    {
        if( matrix[0][0] > matrix[1][1] && matrix[0][0] > matrix[2][2] )
        {
            quat_value_type invs = 0.5 / std::sqrt(1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2]);
            return Quaternion(
                (matrix[2][1] - matrix[1][2]) * invs,
                (quat_value_type)0.25 / invs,
                (matrix[0][1] - matrix[1][0]) * invs,
                (matrix[2][0] - matrix[0][2]) * invs);
        }
        else if( matrix[1][1] > matrix[2][2] )
        {
            quat_value_type invs = 0.5 / std::sqrt(1.0 + matrix[1][1] - matrix[0][0] - matrix[2][2]);
            return Quaternion(
                (matrix[0][2] - matrix[2][0]) * invs,
                (matrix[0][1] - matrix[1][0]) * invs,
                (quat_value_type)0.25 / invs,
                (matrix[1][2] - matrix[2][1]) * invs);
        }
        else
        {
            quat_value_type invs = 0.5 / std::sqrt(1.0 + matrix[2][2] - matrix[0][0] - matrix[1][1]);
            return Quaternion(
                (matrix[1][0] - matrix[0][1]) * invs,
                (matrix[0][2] - matrix[2][0]) * invs,
                (matrix[1][2] + matrix[2][1]) * invs,
                (quat_value_type) 0.25 / invs);
        }
    }
}

Quaternion from_rotation_matrix(const Matrix4T& matrix)
{
    return from_rotation_matrix(hproject(matrix));
}

Matrix3T to_rotation_matrix(const Quaternion& rhs)
{
    return {
        (quat_value_type)(1.0 - 2.0 * rhs[2] * rhs[2] - 2.0 * rhs[3] * rhs[3]),
        (quat_value_type)(2.0 * rhs[1] * rhs[2] - 2.0 * rhs[0] * rhs[3]),
        (quat_value_type)(2.0 * rhs[1] * rhs[3] + 2.0 * rhs[0] * rhs[2]),
        (quat_value_type)(2.0 * rhs[1] * rhs[2] + 2.0 * rhs[0] * rhs[3]),
        (quat_value_type)(1.0 - 2.0 * rhs[1] * rhs[1] - 2.0 * rhs[3] * rhs[3]),
        (quat_value_type)(2.0 * rhs[2] * rhs[3] - 2.0 * rhs[0] * rhs[1]),
        (quat_value_type)(2.0 * rhs[1] * rhs[3] - 2.0 * rhs[0] * rhs[2]),
        (quat_value_type)(2.0 * rhs[2] * rhs[3] + 2.0 * rhs[0] * rhs[1]),
        (quat_value_type)(1.0 - 2.0 * rhs[1] * rhs[1] - 2.0 * rhs[2] * rhs[2])
    };
}

Quaternion lerp(const Quaternion& lhs, const Quaternion& rhs, float t, bool shortest)
{
    quat_value_type vcos = dot(lhs, rhs);
    if( vcos < 0.0 && shortest )
        return normalize( lhs + ((-rhs) - lhs) * t );
    else
        return normalize( lhs + (rhs - lhs) * t);
}

Quaternion slerp(const Quaternion& lhs, const Quaternion& rhs, float t)
{
    Quaternion rhsv = rhs;
    quat_value_type vcos = dot(lhs, rhs);

    if( vcos < 0.0 )
    {
        vcos = -vcos;
        rhsv = -rhs;
    }

    quat_value_type angle   = std::acos(vcos);
    quat_value_type sina    = std::sin(angle);

    if( sina < 0.001 )
    {
        quat_value_type invsa = 1.0 / sina;
        quat_value_type t1 = std::sin((1.0 - t)*angle) * invsa;
        quat_value_type t2 = std::sin(t * angle) * invsa;
        return lhs * t1 + rhsv * t2;
    }
    else
    {
        return lhs * (1.0 - t) + rhs * t;
    }
}

NS_FLOW2D_MATH_END