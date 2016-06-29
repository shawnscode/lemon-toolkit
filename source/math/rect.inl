// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF RECT
template<size_t N, typename T>
Rect<N, T>::Rect() {}

template<size_t N, typename T>
Rect<N, T>::Rect(std::initializer_list<T> values)
{
    auto size = values.size();
    auto cursor = values.begin();

    for( size_t i = 0; i < N; i++ )
        m_position[i] = i < size ? *cursor++ : 0;

    for( size_t i = N; i < 2*N; i++ )
        m_corner[i-N] = i < size ? *cursor++ : m_position[i-N];
}

template<size_t N, typename T>
Rect<N, T>::Rect(const Vector<N, T>& position, const Vector<N, T>& m_corner)
: m_position(position), m_corner(m_corner)
{}

template<size_t N, typename T>
INLINE const T& Rect<N, T>::operator[](int i) const
{
    ENSURE( i >= 0 && i < 2*N );

    if( i < N ) return m_position[i];
    return m_corner[i-N];
}

template<size_t N, typename T>
INLINE T& Rect<N, T>::operator[](int i)
{
    ENSURE( i >= 0 && i < 2*N );

    if( i < N ) return m_position[i];
    return m_corner[i-N];
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator == (const Rect<N, T>& rh) const
{
    return m_position == rh.m_position && m_corner == rh.m_corner;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator != (const Rect<N, T>& rh) const
{
    return m_position != rh.m_position || m_corner != rh.m_corner;
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
    for( size_t i = 0; i < N; i++ )
    {
        if( rh.m_position[i] < m_position[i] )
            m_position[i] = rh.m_position[i];
        if( rh.m_corner[i] > m_corner[i] )
            m_corner[i] = rh.m_corner[i];
    }
    return *this;
}

template<size_t N, typename T>
INLINE Rect<N, T>& Rect<N, T>::operator += (const Vector<N, T>& rh)
{
    for( size_t i = 0; i < N; i++ )
    {
        if( rh[i] < m_position[i] ) m_position[i] = rh[i];
        if( rh[i] > m_corner[i] ) m_corner[i] = rh[i];
    }
    return *this;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::is_inside(const Vector<N, T>& coord) const
{
    for( size_t i = 0; i < N; i++ )
    {
        if( coord[i] < m_position[i] || coord[i] >= m_corner[i] )
            return false;
    }
    return true;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::position() const
{
    return m_position;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::corner() const
{
    return m_corner;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::center() const
{
    return m_position + (m_corner - m_position) * 0.5f;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::size() const
{
    return max(m_corner - m_position, 0.f);
}

template<size_t N, typename T>
INLINE T Rect<N, T>::area() const
{
    T result = (T)1;
    for( size_t i = 0; i < N; i++ )
        result *= std::max(m_corner[i] - m_position[i], (T)0);
    return result;
}

template<size_t N, typename T>
INLINE bool equals(const Rect<N, T>& lh, const Rect<N, T>& rh, T epslion)
{
    return
        equals( lh.position(), rh.position(), epslion ) &&
        equals( lh.size(), rh.size(), epslion );
}

template<size_t N, typename T>
INLINE Rect<N, T> intersect(const Rect<N, T>& lh, const Rect<N, T>& rh)
{
    Vector<N, T> position = max(lh.position(), rh.position());
    Vector<N, T> corner = min(lh.corner(), rh.corner());

    if( corner[0] >= position[0] && corner[1] >= position[1] )
        return Rect<N, T> {position, corner};
    return {(T)0, (T)0, (T)0, (T)0};
}
