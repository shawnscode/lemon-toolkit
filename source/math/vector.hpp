// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/defines.hpp>

NS_FLOW2D_MATH_BEGIN

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
    T _tuple[N];
};

template<typename T>
using Vector2   = Vector<2, T>;
using Vector2f  = Vector<2, float>;
using Vector2i  = Vector<2, int>;

template<typename T>
using Vector3   = Vector<3, T>;
using Vector3f  = Vector<3, float>;
using Vector3i  = Vector<3, int>;

template<typename T>
using Vector4   = Vector<4, T>;
using Vector4f  = Vector<4, float>;
using Vector4i  = Vector<4, int>;

// string serialization with stream
template<size_t N, typename T>
std::ostream& operator<< (std::ostream&, const Vector<N, T>&);

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
Vector<N, T> operator / (const Vector<N, T>&, const Vector<N, T>&);

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
Vector<N, T>& operator /= (Vector<N, T>&, const Vector<N, T>&);

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
Vector<N+1, T> hlift(const Vector<N, T>&, T last = (T)0);
// project homogeneous n-tuple v = (u,v[n-1]) to (n-1)-tuple u.
template<size_t N, typename T>
Vector<N-1, T> hproject(const Vector<N, T>&);

#include <math/vector.inl>
NS_FLOW2D_MATH_END