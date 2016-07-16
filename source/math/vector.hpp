// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<size_t N, typename T> struct Vector
{
    Vector();
    Vector(std::initializer_list<T>);

    Vector(const Vector&) = default;
    Vector& operator = (const Vector&) = default;

    // member access
    const T& operator[](size_t) const;
    T& operator[](size_t);

    // comparisons for sorted containers and geometric ordering without epslion
    bool operator == (const Vector&) const;
    bool operator != (const Vector&) const;
    bool operator <  (const Vector&) const;
    bool operator <= (const Vector&) const;
    bool operator >  (const Vector&) const;
    bool operator >= (const Vector&) const;

    // geometric operations
    T length () const;
    T length_square () const;
    T normalize ();

    void zero();
    void unit(size_t);

protected:
    std::array<T, N> m_tuple;
};

template<typename T>
using Vector2   = Vector<2, T>;
using Vector2f  = Vector<2, float>;
using Vector2i  = Vector<2, int>;

static const Vector2f kVector2fZero = { 0.f, 0.f };
static const Vector2f kVector2fInfinity = { math::inf, math::inf };
static const Vector2f kVector2fNan = { math::nan, math::nan };
static const Vector2f kVector2fOne  = { 1.f, 1.f };
static const Vector2f kVector2fLeft = { -1.f, 0.f };
static const Vector2f kVector2fRight = { 1.f, 0.f };
static const Vector2f kVector2fUp = { 0.f, 1.f };
static const Vector2f kVector2fDown = { 0.f, -1.f };


// unary operations
template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>&);

// linear-algebraic operations
template<size_t N, typename T>
Vector<N, T> operator + (const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> operator - (const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> operator * (const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> operator * (const Vector<N, T>&, T);

template<size_t N, typename T>
Vector<N, T> operator * (T, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> operator / (const Vector<N, T>&, T);

template<size_t N, typename T>
Vector<N, T>& operator += (Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T>& operator -= (Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T>& operator *= (Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T>& operator *= (Vector<N, T>&, T);

template<size_t N, typename T>
Vector<N, T>& operator /= (Vector<N, T>&, T);

// comparison with epslion
template<size_t N, typename T>
bool equals (const Vector<N, T>&, const Vector<N, T>&, T epslion = std::numeric_limits<T>::epsilon());

template<size_t N, typename T>
T dot (const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> max(const Vector<N, T>&, T);

template<size_t N, typename T>
Vector<N, T> max(const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> min(const Vector<N, T>&, T);

template<size_t N, typename T>
Vector<N, T> min(const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> normalize (const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> abs (const Vector<N, T>&);

template<size_t N, typename T>
Vector<N, T> lerp (const Vector<N, T>&, const Vector<N, T>&, float);

template<size_t N, typename T>
Vector<N, T> clamp(const Vector<N, T>&, const Vector<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
bool isnan (const Vector<N, T>&);

template<size_t N, typename T>
bool isinf (const Vector<N, T>&);

// lift n-tuple v to homogeneous (n+1)-tuple (v,last).
template<size_t N, typename T>
Vector<N+1, T> hlift(const Vector<N, T>&, T last = (T)1);
// project homogeneous n-tuple v = (u,v[n-1]) to (n-1)-tuple u.
template<size_t N, typename T>
Vector<N-1, T> hproject(const Vector<N, T>&);

#include <math/vector.inl>
NS_FLOW2D_END