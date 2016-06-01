// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<size_t N, typename T> struct Vector
{
    Vector();
    Vector(const std::array<T, N>&);
    Vector(std::initializer_list<T>);

    Vector(const Vector&) = default;
    Vector& operator = (const Vector&) = default;

    INLINE const T& operator[](int) const;
    INLINE T& operator[](int);

    // comparisons for sorted containers and geometric ordering
    INLINE bool operator == (const Vector&) const;
    INLINE bool operator != (const Vector&) const;
    INLINE bool operator <  (const Vector&) const;
    INLINE bool operator <= (const Vector&) const;
    INLINE bool operator >  (const Vector&) const;
    INLINE bool operator >= (const Vector&) const;

protected:
    std::array<T, N> m_tuple;
};

template<size_t N, typename T> Vector<N, T> operator + (const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T> operator - (const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T> operator + (const Vector<N, T>&, const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T> operator - (const Vector<N, T>&, const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T> operator * (const Vector<N, T>&, T);
template<size_t N, typename T> Vector<N, T> operator / (const Vector<N, T>&, T);

template<size_t N, typename T> Vector<N, T>& operator += (Vector<N, T>&, const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T>& operator -= (Vector<N, T>&, const Vector<N, T>&);
template<size_t N, typename T> Vector<N, T>& operator *= (Vector<N, T>&, T);
template<size_t N, typename T> Vector<N, T>& operator /= (Vector<N, T>&, T);

template<size_t N, typename T> T dot (const Vector<N, T>&, const Vector<N, T>&);
template<size_t N, typename T> T length (const Vector<N, T>&);
template<size_t N, typename T> T length_square (const Vector<N, T>&);
template<size_t N, typename T> T normalize (Vector<N, T>&);

using Vector2f = Vector<2, float>;
using Vector2i = Vector<2, int>;

#include "vector.inl"
NS_FLOW2D_END