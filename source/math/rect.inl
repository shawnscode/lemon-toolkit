// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF RECT
template<size_t N, typename T>
Rect<N, T>::Rect()
: min({inf<T>(), inf<T>()}), max({-inf<T>(), -inf<T>()})
{}

template<size_t N, typename T>
Rect<N, T>::Rect(const Vector<N, T>& min, const Vector<N, T>& max)
: min(min), max(lemon::math::max(min, max))
{}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator == (const Rect<N, T>& rhs) const
{
    return min == rhs.min && max == rhs.max;
}

template<size_t N, typename T>
INLINE bool Rect<N, T>::operator != (const Rect<N, T>& rhs) const
{
    return min != rhs.min || max != rhs.max;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::center() const
{
    return min + (max - min) * 0.5f;
}

template<size_t N, typename T>
INLINE Vector<N, T> Rect<N, T>::size() const
{
    return max(max - min, 0.f);
}

template<size_t N, typename T>
template<size_t A>
INLINE T Rect<N, T>::length() const
{
    static_assert( A < N, "invalid axis." );
    return std::max(max[A] - min[A], (T)0);
}

template<size_t N, typename T>
INLINE bool equals(const Rect<N, T>& lhs, const Rect<N, T>& rhs, T epslion)
{
    return equals( lhs.min, rhs.min, epslion ) && equals( lhs.max, rhs.max, epslion );
}

template<size_t N, typename T>
INLINE Rect<N, T> intersect(const Rect<N, T>& lhs, const Rect<N, T>& rhs)
{
    Rect<N, T> result = lhs;
    if( rhs.min[0] > lhs.min[0] ) result.min[0] = rhs.min[0];
    if( rhs.max[0] < lhs.max[0] ) result.max[0] = rhs.max[0];
    if( rhs.min[1] > lhs.min[1] ) result.min[1] = rhs.min[1];
    if( rhs.max[1] < lhs.max[1] ) result.max[1] = rhs.max[1];

    if( result.min[0] > result.max[0] || result.min[1] > result.max[1] )
    {
        result.min = { inf<float>(), inf<float>() };
        result.max = { -inf<float>(), inf<float>() };
    }

    return result;
}

template<size_t N, typename T>
INLINE Rect<N, T> merge(const Rect<N, T>& lhs, const Rect<N, T>& rhs)
{
    Rect<N, T> result = lhs;
    for( size_t i = 0; i < N; i++ )
    {
        if( rhs.min[i] < lhs.min[i] ) result.min[i] = rhs.min[i];
        if( rhs.max[i] > lhs.max[i] ) result.max[i] = rhs.max[i];
    }
    return result;
}

template<size_t N, typename T>
INLINE Rect<N, T> merge(const Rect<N, T>& lhs, const Vector<N, T>& rhs)
{
    Rect<N, T> result = lhs;
    for( size_t i = 0; i < N; i++ )
    {
        if( rhs[i] < lhs.min[i] ) result.min[i] = rhs[i];
        if( rhs[i] > lhs.max[i] ) result.max[i] = rhs[i];
    }
    return result;
}

template<size_t N, typename T>
INLINE bool is_inside(const Rect<N, T>& rect, const Vector<N, T>& coord)
{
    for( size_t i = 0; i < N; i++ )
        if( coord[i] < rect.min[i] || coord[i] >= rect.max[i] )
            return false;
    return true;
}

template<size_t N, typename T>
INLINE bool isnan(const Rect<N, T>& rect)
{
    for( size_t i = 0; i < N; i++ )
        if( rect.min[i] > rect.max[i] ) return true;
    return false;
}

template<size_t N, typename T>
INLINE Vector<N*2, T> to_vector(const Rect<N, T>& rect)
{
    Vector<N*2, T> result;
    for( size_t i = 0; i < N; i++ ) result[i] = rect.min[i];
    for( size_t i = 0; i < N; i++ ) result[i+N] = rect.max[i];
    return result;
}