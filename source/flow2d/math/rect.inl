// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF RECT

template<typename T> const Rect2<T> Rect2<T>::FULL { -1, -1, 1, 1 };
template<typename T> const Rect2<T> Rect2<T>::POSITIVE { 0, 0, 1, 1 };

template<typename T>
Rect2<T>::Rect2() {}

template<typename T>
Rect2<T>::Rect2(std::initializer_list<T> values)
{
    auto cursor = values.begin();
    if( values.size() >= 1 ) m_min[0] = *cursor++;
    if( values.size() >= 2 ) m_min[1] = *cursor++;
    if( values.size() >= 3 ) m_max[0] = *cursor++;
    if( values.size() >= 4 ) m_max[1] = *cursor++;
}

template<typename T>
INLINE const Vector2<T>& Rect2<T>::operator[](int i) const
{
    assert( i >= 0 && i < 2 );

    if( i == 0 ) return m_min;
    return m_max;
}

template<typename T>
INLINE Vector2<T>& Rect2<T>::operator[](int i)
{
    assert( i >= 0 && i < 2 );

    if( i == 0 ) return m_min;
    return m_max;
}

template<typename T>
INLINE bool Rect2<T>::operator == (const Rect2<T>& rh) const
{
    return m_min == rh.m_min && m_max == rh.m_max;
}

template<typename T>
INLINE bool Rect2<T>::operator != (const Rect2<T>& rh) const
{
    return m_min != rh.m_min || m_max != rh.m_max;
}

template<typename T>
INLINE Rect2<T> Rect2<T>::operator + (const Rect2<T>& rh) const
{
    auto result = *this;
    return result += rh;
}

template<typename T>
INLINE Rect2<T> Rect2<T>::operator + (const Vector2<T>& rh) const
{
    auto result = *this;
    return result += rh;
}

template<typename T>
INLINE Rect2<T>& Rect2<T>::operator += (const Rect2<T>& rh)
{
    if( rh.m_min[0] < m_min[0] ) m_min[0] = rh.m_min[0];
    if( rh.m_min[1] < m_min[1] ) m_min[1] = rh.m_min[1];

    if( rh.m_max[0] > m_max[0] ) m_max[0] = rh.m_max[0];
    if( rh.m_max[1] > m_max[1] ) m_max[1] = rh.m_max[1];

    return *this;
}

template<typename T>
INLINE Rect2<T>& Rect2<T>::operator += (const Vector2<T>& rh)
{
    if( rh[0] < m_min[0] ) m_min[0] = rh[0];
    if( rh[0] > m_max[0] ) m_max[0] = rh[0];

    if( rh[1] < m_min[1] ) m_min[1] = rh[1];
    if( rh[1] > m_min[1] ) m_max[1] = rh[1];

    return *this;
}

template<typename T>
INLINE bool Rect2<T>::equals(const Rect2& rh, T epslion) const
{
    return m_min.equals(rh.m_min, epslion) && m_max.equals(rh.m_max, epslion);
}

template<typename T>
INLINE bool Rect2<T>::is_inside(const Vector2<T>& coord) const
{
    return (coord[0] >= m_min[0]) && (coord[0] < m_max[0]) && (coord[1] >= m_min[1]) && (coord[1] < m_max[1]);
}

template<typename T>
INLINE T Rect2<T>::x() const
{
    return m_min[0];
}

template<typename T>
INLINE T Rect2<T>::y() const
{
    return m_min[1];
}

template<typename T>
INLINE T Rect2<T>::width() const
{
    return m_max[0] - m_min[0];
}
template<typename T>
INLINE T Rect2<T>::height() const
{
    return m_max[1] - m_min[1];
}

template<typename T>
INLINE Vector2<T> Rect2<T>::center() const
{
    return m_min + (m_max - m_min) * 0.5;
}

template<typename T>
INLINE Vector2<T> Rect2<T>::size() const
{
    return m_max - m_min;
}

template<typename T>
INLINE Vector2<T> Rect2<T>::half_size() const
{
    return (m_max - m_min) * 0.5;
}