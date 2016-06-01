// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<typename T> struct Vector2
{
    Vector2();
    Vector2(std::initializer_list<T>);

    Vector2(const Vector2&) = default;
    Vector2& operator = (const Vector2&) = default;

    const T& operator[](int) const;
    T& operator[](int);

    // comparisons for sorted containers and geometric ordering without epslion
    bool operator == (const Vector2&) const;
    bool operator != (const Vector2&) const;
    bool operator <  (const Vector2&) const;
    bool operator <= (const Vector2&) const;
    bool operator >  (const Vector2&) const;
    bool operator >= (const Vector2&) const;

    Vector2<T> operator + () const;
    Vector2<T> operator - () const;
    Vector2<T> operator + (const Vector2<T>&) const;
    Vector2<T> operator - (const Vector2<T>&) const;
    Vector2<T> operator * (T) const;
    Vector2<T> operator / (T) const;

    Vector2<T>& operator += (const Vector2<T>&);
    Vector2<T>& operator -= (const Vector2<T>&);
    Vector2<T>& operator *= (T);
    Vector2<T>& operator /= (T);

    // 
    bool equals (const Vector2&, T epslion = std::numeric_limits<T>::epsilon()) const;
    bool isnan () const;
    bool isinf () const;

    Vector2<T> normalize () const;
    Vector2<T> abs () const;
    Vector2<T> lerp (const Vector2<T>&, float) const;

    T dot (const Vector2<T>&) const;
    T length () const;
    T length_square () const;

    static const Vector2<T> ZERO;
    static const Vector2<T> ONE;
    static const Vector2<T> LEFT;
    static const Vector2<T> RIGHT;
    static const Vector2<T> UP;
    static const Vector2<T> DOWN;

protected:
    std::array<T, 2> m_tuple;
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

#include "vector.inl"
NS_FLOW2D_END