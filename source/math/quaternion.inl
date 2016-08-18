// @date 2016/08/15
// @author Mao Jingkai(oammix@gmail.com)

INLINE std::ostream& operator << (std::ostream& out, const Quaternion& rhs)
{
    return out
        << "Quaternion("
        << rhs[0] << ", "
        << rhs[1] << ","
        << rhs[2] << ","
        << rhs[3] << ")";
}

INLINE const quat_value_type& Quaternion::operator[](size_t index) const
{
    return _tuple[index];
}

INLINE quat_value_type& Quaternion::operator[](size_t index)
{
    return _tuple[index];
}

INLINE bool operator == (const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

INLINE bool operator != (const Quaternion& lhs, const Quaternion& rhs)
{
    return !(lhs == rhs);
}

INLINE Quaternion operator + (const Quaternion& lhs)
{
    return lhs;
}

INLINE Quaternion operator - (const Quaternion& lhs)
{
    return Quaternion(lhs[0], -lhs[1], -lhs[2], -lhs[3]);
}

INLINE Quaternion operator + (const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion(lhs[0]+rhs[0], lhs[1]+rhs[1], lhs[2]+rhs[2], lhs[3]+rhs[3]);
}

INLINE Quaternion operator - (const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs + (-rhs);
}

INLINE Quaternion operator * (const Quaternion& lhs, const Quaternion& rhs)
{
    // hamilton product
    return Quaternion(
        lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2] - lhs[3] * rhs[3],
        lhs[0] * rhs[1] + lhs[1] * rhs[0] + lhs[2] * rhs[3] - lhs[3] * rhs[2],
        lhs[0] * rhs[2] + lhs[2] * rhs[0] + lhs[3] * rhs[1] - lhs[1] * rhs[3],
        lhs[0] * rhs[3] + lhs[3] * rhs[0] + lhs[1] * rhs[2] - lhs[2] * rhs[1]
    );
}

INLINE Quaternion operator / (const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs * inverse(rhs);
}

INLINE Quaternion operator * (const Quaternion& lhs, quat_value_type v)
{
    return Quaternion(lhs[0]*v, lhs[1]*v, lhs[2]*v, lhs[3]*v);
}

INLINE Quaternion& operator += (Quaternion& lhs, const Quaternion& rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

INLINE Quaternion& operator -= (Quaternion& lhs, const Quaternion& rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

INLINE Quaternion& operator *= (Quaternion& lhs, const Quaternion& rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

INLINE Quaternion& operator /= (Quaternion& lhs, const Quaternion& rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

INLINE Quaternion& operator *= (Quaternion& lhs, quat_value_type rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

INLINE Vector3T operator * (const Vector3T& lhs, const Quaternion& rhs)
{
    Vector3T qvec = {rhs[1], rhs[2], rhs[3]};
    Vector3T cross1 = cross(qvec, lhs);
    Vector3T cross2 = cross(qvec, cross1);

    return lhs + (quat_value_type)2.0 * (cross1 * rhs[0] + cross2);
}

INLINE Vector3T operator / (const Vector3T& lhs, const Quaternion& rhs)
{
    return lhs * inverse(rhs);
}

INLINE bool equals(const Quaternion& lhs, const Quaternion& rhs, quat_value_type epsilon)
{
    return
        std::abs(lhs[0]-rhs[0]) < epsilon &&
        std::abs(lhs[1]-rhs[1]) < epsilon &&
        std::abs(lhs[2]-rhs[2]) < epsilon &&
        std::abs(lhs[3]-rhs[3]) < epsilon;
}

INLINE quat_value_type length_square(const Quaternion& rhs)
{
    return dot(rhs, rhs);
}

INLINE quat_value_type dot(const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3];
}

INLINE Quaternion conjugate(const Quaternion& rhs)
{
    return Quaternion(rhs[0], -rhs[1], -rhs[2], -rhs[3]);
}

INLINE Quaternion normalize(const Quaternion& rhs)
{
    auto result = rhs;
    quat_value_type slen = length_square(result);
    if( !(std::abs(slen - (quat_value_type)1) < epsilon<quat_value_type>()) &&
        slen > (quat_value_type)0 )
    {
        quat_value_type inv_slen = (quat_value_type)1 / std::sqrt(inv_slen);
        result[0] *= inv_slen;
        result[1] *= inv_slen;
        result[2] *= inv_slen;
        result[3] *= inv_slen;
    }
    return result;
}

INLINE Quaternion inverse(const Quaternion& rhs)
{
    quat_value_type slen = length_square(rhs);
    if( slen == (quat_value_type)1 )
        return conjugate(rhs);
    else if( slen >= epsilon<quat_value_type>() )
        return conjugate(rhs) * ((quat_value_type)1/slen);
    else
        return Quaternion();
}

INLINE quat_value_type angle(const Quaternion& rhs)
{
    return to_degree(std::acos(rhs[0]) * quat_value_type(2));
}

INLINE quat_value_type pitch(const Quaternion& rhs)
{
    quat_value_type y = quat_value_type(2) * (rhs[2] * rhs[3] + rhs[0] * rhs[1]);
    quat_value_type x = rhs[0] * rhs[0] - rhs[1] * rhs[1] - rhs[2] * rhs[2] + rhs[3] * rhs[3];
    return to_degree(quat_value_type(std::atan2(y, x)));
}

INLINE quat_value_type roll(const Quaternion& rhs)
{
    quat_value_type y = quat_value_type(2) * (rhs[1] * rhs[2] + rhs[0] * rhs[3]);
    quat_value_type x = rhs[0] * rhs[0] + rhs[1] * rhs[1] - rhs[2] * rhs[2] - rhs[3] * rhs[3];
    return to_degree(quat_value_type(std::atan2(y, x)));
}

INLINE quat_value_type yaw(const Quaternion& rhs)
{
    quat_value_type v = clamp(quat_value_type(-2) * (rhs[1] * rhs[3] - rhs[0] * rhs[2]), quat_value_type(-1), quat_value_type(1));
    return to_degree(std::asin(v));
}

INLINE Vector3T to_euler_angles(const Quaternion& rhs)
{
    return { pitch(rhs), yaw(rhs), roll(rhs) };
}