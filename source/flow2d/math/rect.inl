// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF RECT
template<size_t N, typename T>
Rect<N, T>::Rect() {}

template<size_t N, typename T>
Rect<N, T>::Rect(std::initializer_list<T> values)
{
    m_position.zero();
    m_size.zero();

    auto cursor = values.begin();
    if( values.size() >= 1 ) m_position[0]  = *cursor++;
    if( values.size() >= 2 ) m_position[1]  = *cursor++;
    if( values.size() >= 3 ) m_size[0]      = *cursor++;
    if( values.size() >= 4 ) m_size[1]      = *cursor++;
}

template<size_t N, typename T>
INLINE const Vector<N, T>& Rect<N, T>::operator[](int i) const
{
    ENSURE( i >= 0 && i < 2 );

    if( i == 0 ) return m_position;
    return m_size;
}

template<size_t N, typename T>
INLINE Vector<N, T>& Rect<N, T>::operator[](int i)
{
    ENSURE( i >= 0 && i < 2 );

    if( i == 0 ) return m_position;
    return m_size;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator == (const Rect<N, T>& rh) const
{
    return m_position == rh.m_position && m_size == rh.m_size;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator != (const Rect<N, T>& rh) const
{
    return m_position != rh.m_position || m_size != rh.m_size;
}

template<size_t N, typename T>
INLINE Rect<N, T> Rect<N, T>::operator + (const Rect<N, T>& rh) const
{
    auto result = *this;
    return result += rh;
}

template<size_t N, typename T>
INLINE Rect<N, T> Rect<N, T>::operator + (const Vector<N, T>& rh) const
{
    auto result = *this;
    return result += rh;
}

template<size_t N, typename T>
INLINE Rect<N, T>& Rect<N, T>::operator += (const Rect<N, T>& rh)
{
    auto ru = m_position + m_size;
    auto other_ru = rh.m_position + rh.m_size;

    if( rh.m_position[0] < m_position[0] ) m_position[0] = rh.m_position[0];
    if( rh.m_position[1] < m_position[1] ) m_position[1] = rh.m_position[1];
    if( other_ru[0] > ru[0] ) ru[0] = other_ru[0];
    if( other_ru[1] > ru[1] ) ru[1] = other_ru[1];

    m_size = ru - m_position;
    return *this;
}

template<size_t N, typename T>
INLINE Rect<N, T>& Rect<N, T>::operator += (const Vector<N, T>& rh)
{
    auto ru = m_position + m_size;

    if( rh[0] < m_position[0] ) m_position[0] = rh[0];
    if( rh[1] < m_position[1] ) m_position[1] = rh[1];
    if( rh[0] > ru[0] ) ru[0] = rh[0];
    if( rh[1] > ru[1] ) ru[1] = rh[1];

    m_size = ru - m_position;
    return *this;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::is_inside(const Vector<N, T>& coord) const
{
    auto ru = m_position + m_size;
    return
        (coord[0] >= m_position[0]) && (coord[0] < ru[0]) &&
        (coord[1] >= m_position[1]) && (coord[1] < ru[1]);
}

template<size_t N, typename T>
INLINE T Rect<N, T>::width() const
{
    return m_size[0];
}
template<size_t N, typename T>
INLINE T Rect<N, T>::height() const
{
    return m_size[1];
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::center() const
{
    return m_position + m_size * 0.5;

}

template<size_t N, typename T>
INLINE const Vector<N, T>& Rect<N, T>::position() const
{
    return m_position;
}

template<size_t N, typename T>
INLINE const Vector<N, T>& Rect<N, T>::size() const
{
    return m_size;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::half_size() const
{
    return m_size * 0.5;
}

template<size_t N, typename T>
INLINE bool equals(const Rect<N, T>& lh, const Rect<N, T>& rh, T epslion)
{
    return
        equals( lh.position(), rh.position(), epslion ) &&
        equals( lh.size(), rh.size(), epslion );
}

template<size_t N, typename T>
INLINE Rect2f clamp(const Rect<N, T>& lh, const Rect<N, T>& rh)
{
    Rect2f result = lh;
    auto ru = lh[0] + lh[1];
    auto other_ru = rh[0] + rh[1];

    if( lh.position()[0] < rh.position()[0] ) result[0][0] = rh[0][0];
    if( lh.position()[1] < rh.position()[1] ) result[0][1] = rh[0][1];

    if( ru[0] > other_ru[0] ) ru[0] = other_ru[0];
    if( ru[1] > other_ru[1] ) ru[1] = other_ru[1];

    result[1] = ru - result[0];
    return result;
}
