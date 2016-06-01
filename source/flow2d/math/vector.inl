// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF VECTOR

template<typename T> const Vector2<T> Vector2<T>::ZERO  { 0, 0 };
template<typename T> const Vector2<T> Vector2<T>::ONE   { 1, 1 };
template<typename T> const Vector2<T> Vector2<T>::LEFT  { -1, 0 };
template<typename T> const Vector2<T> Vector2<T>::RIGHT { 1, 0 };
template<typename T> const Vector2<T> Vector2<T>::UP    { 0, 1 };
template<typename T> const Vector2<T> Vector2<T>::DOWN  { 0, -1 };

template<typename T>
Vector2<T>::Vector2()
{
    m_tuple[0] = m_tuple[1] = static_cast<T>(0);
}

template<typename T>
Vector2<T>::Vector2(std::initializer_list<T> values)
{
    auto i = 0;
    for( auto value : values )
        if( i < 2 ) m_tuple[i++] = value;

    for( ; i < 2; i++ )
        m_tuple[i] = static_cast<T>(0);
}

template<typename T>
INLINE const T& Vector2<T>::operator[] (int i) const
{
    return m_tuple[i];
}

template<typename T>
INLINE T& Vector2<T>::operator[] (int i)
{
    return m_tuple[i];
}

template<typename T>
INLINE bool Vector2<T>::operator == (const Vector2& rh) const
{
    return m_tuple == rh.m_tuple;
}

template<typename T>
INLINE bool Vector2<T>::operator != (const Vector2& rh) const
{
    return m_tuple != rh.m_tuple;
}

template<typename T>
INLINE bool Vector2<T>::operator < (const Vector2& rh) const
{
    return m_tuple < rh.m_tuple;
}

template<typename T>
INLINE bool Vector2<T>::operator <= (const Vector2& rh) const
{
    return m_tuple <= rh.m_tuple;
}

template<typename T>
INLINE bool Vector2<T>::operator > (const Vector2& rh) const
{
    return m_tuple > rh.m_tuple;
}

template<typename T>
INLINE bool Vector2<T>::operator >= (const Vector2& rh) const
{
    return m_tuple >= rh.m_tuple;
}

////
template<typename T>
INLINE Vector2<T> Vector2<T>::operator + () const
{
    return *this;
}

template<typename T>
INLINE Vector2<T> Vector2<T>::operator - () const
{
    return Vector2<T> { -m_tuple[0], -m_tuple[1] };
}

template<typename T>
INLINE Vector2<T> Vector2<T>::operator + (const Vector2<T>& rh) const
{
    return Vector2<T> { m_tuple[0] + rh.m_tuple[0], m_tuple[1] + rh.m_tuple[1] };
}

template<typename T>
INLINE Vector2<T> Vector2<T>::operator - (const Vector2<T>& rh) const
{
    return Vector2<T> { m_tuple[0] - rh.m_tuple[0], m_tuple[1] - rh.m_tuple[1] };
}

template<typename T>
INLINE Vector2<T> Vector2<T>::operator * (T rh) const
{
    return Vector2<T> { m_tuple[0] * rh, m_tuple[1] * rh };
}

template<typename T>
INLINE Vector2<T> Vector2<T>::operator / (T rh) const
{
    auto inv = 1.0 / rh;
    return *this * inv;
}


template<typename T>
INLINE Vector2<T>& Vector2<T>::operator += (const Vector2<T>& rh)
{
    m_tuple[0] += rh.m_tuple[0];
    m_tuple[1] += rh.m_tuple[1];
    return *this;
}

template<typename T>
INLINE Vector2<T>& Vector2<T>::operator -= (const Vector2<T>& rh)
{
    m_tuple[0] -= rh.m_tuple[0];
    m_tuple[1] -= rh.m_tuple[1];
    return *this;
}

template<typename T>
INLINE Vector2<T>& Vector2<T>::operator *= (T rh)
{
    m_tuple[0] *= rh;
    m_tuple[1] *= rh;
    return *this;
}

template<typename T>
INLINE Vector2<T>& Vector2<T>::operator /= (T rh)
{
    T invrh = 1.0 / rh;
    return m_tuple *= invrh;
}

template<typename T>
INLINE bool Vector2<T>::equals (const Vector2& to, T epslion) const
{
    return std::abs(m_tuple[0]-to.m_tuple[0]) < epslion && std::abs(m_tuple[1]-to.m_tuple[1]) < epslion;
}

template<typename T>
INLINE bool Vector2<T>::isnan () const
{
    return std::isnan(m_tuple[0]) || std::isnan(m_tuple[1]);
}

template<typename T>
INLINE bool Vector2<T>::isinf() const
{
    return std::isinf(m_tuple[0]) || std::isinf(m_tuple[1]);
}

template<typename T>
INLINE Vector2<T> Vector2<T>::normalize () const
{
    return (*this) / length();
}

template<typename T>
INLINE Vector2<T> Vector2<T>::abs () const
{
    return Vector2<T> { std::abs(m_tuple[0]), std::abs(m_tuple[1]) };
}

template<typename T>
INLINE Vector2<T> Vector2<T>::lerp (const Vector2<T>& to, float t) const
{
    if( t >= 1.0f ) return to;
    if( t <= 0.0f ) return *this;
    return (*this) + (to-(*this)) * t;
}

template<typename T>
INLINE T Vector2<T>::dot (const Vector2<T>& rh) const
{
    return m_tuple[0]*rh.m_tuple[0] + m_tuple[1]*rh.m_tuple[1];
}

template<typename T>
INLINE T Vector2<T>::length () const
{
    return std::sqrt(dot(*this));
}

template<typename T>
INLINE T Vector2<T>::length_square () const
{
    return dot(*this);
}
