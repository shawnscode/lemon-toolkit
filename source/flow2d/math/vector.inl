// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF VECTOR

template<size_t N, typename T>
Vector<N, T>::Vector() {}

template<size_t N, typename T>
Vector<N, T>::Vector(std::initializer_list<T> values)
{
    size_t i = 0;
    for( auto value : values )
    {
        if( i < N ) m_tuple[i++] = value;
        else break;
    }

    for( ; i < N; ++i ) m_tuple[i] = (T)0;
}

template<size_t N, typename T>
INLINE const T& Vector<N, T>::operator[] (size_t i) const
{
    return m_tuple[i];
}

template<size_t N, typename T>
INLINE T& Vector<N, T>::operator[] (size_t i)
{
    return m_tuple[i];
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator == (const Vector<N, T>& rh) const
{
    return m_tuple == rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator != (const Vector<N, T>& rh) const
{
    return m_tuple != rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator < (const Vector<N, T>& rh) const
{
    return m_tuple < rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator <= (const Vector<N, T>& rh) const
{
    return m_tuple <= rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator > (const Vector<N, T>& rh) const
{
    return m_tuple > rh.m_tuple;
}

template<size_t N, typename T>
INLINE bool Vector<N, T>::operator >= (const Vector<N, T>& rh) const
{
    return m_tuple >= rh.m_tuple;
}

template<size_t N, typename T>
INLINE void Vector<N, T>::zero()
{
    std::fill(m_tuple.begin(), m_tuple.end(), (T)0);
}

template<size_t N, typename T>
INLINE void Vector<N, T>::unit(size_t d)
{
    zero();
    if( 0 <= d && d < N ) m_tuple[d] = (T)1;
}

template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>& v)
{
    return v;
}

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>& v)
{
    Vector<N, T> result;
    for( auto i = 0; i < N; ++i ) result[i] = -v[i];
    return result;
}

// linear-algebraic operations
template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>& v0, const Vector<N, T>& v1)
{
    Vector<N, T> result = v0;
    return result += v1;
}

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>& v0, const Vector<N, T>& v1)
{
    Vector<N, T> result = v0;
    return result -= v1;
}

template<size_t N, typename T>
Vector<N, T> operator * (const Vector<N, T>& v, T scalar)
{
    Vector<N, T> result = v;
    return result *= scalar;
}

template<size_t N, typename T>
Vector<N, T> operator / (const Vector<N, T>& v, T scalar)
{
    Vector<N, T> result = v;
    return result /= scalar;
}

template<size_t N, typename T>
Vector<N, T>& operator += (Vector<N, T>& v0, const Vector<N, T>& v1)
{
    for( auto i = 0; i < N; i++ ) v0[i] += v1[i];
    return v0;
}

template<size_t N, typename T>
Vector<N, T>& operator -= (Vector<N, T>& v0, const Vector<N, T>& v1)
{
    for( auto i = 0; i < N; i++ ) v0[i] -= v1[i];
    return v0;
}

template<size_t N, typename T>
Vector<N, T>& operator *= (Vector<N, T>& v, T scalar)
{
    for( auto i = 0; i < N; i++ ) v[i] *= scalar;
    return v;
}

template<size_t N, typename T>
Vector<N, T>& operator /= (Vector<N, T>& v, T scalar)
{
    for( auto i = 0; i < N; i++ ) v[i] /= scalar;
    return v;
}

template<size_t N, typename T>
T Vector<N, T>::length () const
{
    return std::sqrt(dot(*this, *this));
}

template<size_t N, typename T>
T Vector<N, T>::length_square () const
{
    return dot(*this, *this);
}

template<size_t N, typename T>
T Vector<N, T>::normalize ()
{
    T len = length();
    *this /= len;
    return len;
}

template<size_t N, typename T>
bool equals (const Vector<N, T>& v0, const Vector<N, T>& v1, T epslion)
{
    for( auto i = 0; i < N; i++ )
        if( std::abs(v0[i] - v1[i]) >= epslion ) return false;
    return true;
}

template<size_t N, typename T>
T dot (const Vector<N, T>& v0, const Vector<N, T>& v1)
{
    T result = (T)0;
    for( auto i = 0; i < N; i++ ) result += v0[i] * v1[i];
    return result;
}

template<size_t N, typename T>
Vector<N, T> normalize (const Vector<N, T>& v)
{
    Vector<N, T> result = v;
    T len = v.length();
    result /= len;
    return result;
}

template<size_t N, typename T>
Vector<N, T> abs (const Vector<N, T>& v)
{
    Vector<N, T> result = v;
    for( auto i = 0; i < N; i++ ) result[i] = std::abs(v[i]);
    return result;
}

template<size_t N, typename T>
Vector<N, T> lerp (const Vector<N, T>& v0, const Vector<N, T>& v1, float ratio)
{
    if( ratio >= 1.0f ) return v1;
    if( ratio <= 0.0f ) return v0;
    return v0 + (v1-v0) * ratio;
}

template<size_t N, typename T>
Vector<N, T> clamp (const Vector<N, T>& source, const Vector<N, T>& v0, const Vector<N, T>& v1)
{
    ENSURE( v0[0] <= v1[0] && v0[1] <= v1[1] );

    Vector<N, T> result = source;
    if( result[0] < v0[0] ) result[0] = v0[0];
    if( result[1] < v0[1] ) result[1] = v0[1];
    if( result[0] > v1[0] ) result[0] = v1[0];
    if( result[1] > v1[1] ) result[1] = v1[1];
    return result;
}

template<size_t N, typename T>
bool isnan (const Vector<N, T>& v)
{
    for( auto i = 0; i < N; i++ )
        if( std::isnan(v[i]) ) return true;
    return false;
}

template<size_t N, typename T>
bool isinf (const Vector<N, T>& v)
{
    for( auto i = 0; i < N; i++ )
        if( std::isinf(v[i]) ) return true;
    return false;
}