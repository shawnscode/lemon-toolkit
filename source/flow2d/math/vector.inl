// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF VECTOR

template<size_t N, typename T>
Vector<N, T>::Vector()
{}

template<size_t N, typename T>
Vector<N, T>::Vector(const std::array<T, N>& values)
: m_tuple(values)
{}

template<size_t N, typename T>
Vector<N, T>::Vector(std::initializer_list<T> values)
{
    auto i = 0;
    for( auto value : values )
    {
        if( i < N ) m_tuple[i++] = value;
        else break;
    }

    for( ; i<N; i++ )
        m_tuple[i] = static_cast<T>(0);
}

template<size_t N, typename T>
INLINE const T& Vector<N, T>::operator[] (int i) const
{
    return m_tuple[i];
}

template<size_t N, typename T>
INLINE T& Vector<N, T>::operator[] (int i)
{
    return m_tuple[i];
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator == (const Vector& rh) const
{
    return m_tuple == rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator != (const Vector& rh) const
{
    return m_tuple != rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator < (const Vector& rh) const
{
    return m_tuple < rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator <= (const Vector& rh) const
{
    return m_tuple <= rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator > (const Vector& rh) const
{
    return m_tuple > rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator >= (const Vector& rh) const
{
    return m_tuple >= rh.m_tuple;
}

////
template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>& rh)
{
    return rh;
}

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>& rh)
{
    Vector<N, T> result;
    for( auto i=0; i<N; i++ )
        result[i] = -rh[i];
    return result;
}

template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>& lh, const Vector<N, T>& rh)
{
    auto result = lh;
    return result += rh;
}

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>& lh, const Vector<N, T>& rh)
{
    auto result = lh;
    return result -= rh;
}

template<size_t N, typename T>
Vector<N, T> operator * (const Vector<N, T>& lh, T rh)
{
    auto result = lh;
    return result *= rh;
}

template<size_t N, typename T>
Vector<N, T> operator / (const Vector<N, T>& lh, T rh)
{
    auto result = lh;
    return result /= rh;
}


template<size_t N, typename T>
Vector<N, T>& operator += (Vector<N, T>& lh, const Vector<N, T>& rh)
{
    for( auto i=0; i<N; i++ )
        lh[i] += rh[i];
    return lh;
}

template<size_t N, typename T>
Vector<N, T>& operator -= (Vector<N, T>& lh, const Vector<N, T>& rh)
{
    for( auto i=0; i<N; i++ )
        lh[i] -= rh[i];
    return lh;
}

template<size_t N, typename T>
Vector<N, T>& operator *= (Vector<N, T>& lh, T rh)
{
    for( auto i=0; i<N; i++ )
        lh[i] *= rh;
    return lh;
}

template<size_t N, typename T>
Vector<N, T>& operator /= (Vector<N, T>& lh, T rh)
{
    T invrh = 1.0 / rh;
    return lh *= invrh;
}

template<size_t N, typename T>
T dot (const Vector<N, T>& lh, const Vector<N, T>& rh)
{
    T result = 0;
    for (auto i = 0; i < N; ++i)
        result += lh[i] * rh[i];
    return result;
}

template<size_t N, typename T>
T length (const Vector<N, T>& lh)
{
    return sqrt(dot(lh, lh));
}

template<size_t N, typename T>
T length_square (const Vector<N, T>& lh)
{
    return dot(lh, lh);
}

template<size_t N, typename T>
T normalize (Vector<N, T>& lh)
{
    T result = length(lh);
    lh /= result;
    return result;
}
