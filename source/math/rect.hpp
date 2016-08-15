// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/vector.hpp>

NS_FLOW2D_MATH_BEGIN

template<size_t N, typename T> struct Rect
{
    // construct an undefined rect
    Rect();
    // construct from minimum and maximum position
    Rect(const Vector<N, T>&, const Vector<N, T>&);

    Rect(const Rect&) = default;
    Rect& operator = (const Rect&) = default;

    // test for equality/inequality with another rect
    bool operator == (const Rect<N, T>&) const;
    bool operator != (const Rect<N, T>&) const;

    // return center
    Vector<N, T> center() const;
    // return size
    Vector<N, T> size() const;
    // return minimum/maximum/length in specfied axis
    template<size_t Axis> T length() const;

    Vector<N, T> min, max;
};

template<typename T>
using Rect2  = Rect<2, T>;
using Rect2f = Rect<2, float>;
using Rect2i = Rect<2, int>;

// string serialization with stream
template<size_t N, typename T>
std::ostream& operator<< (std::ostream&, const Rect<N, T>&);

// test for equality with another rect with epsilon
template<size_t N, typename T>
bool equals(const Rect<N, T>&, const Rect<N, T>&, T epslion = std::numeric_limits<T>::epsilon());

// clip with another rect.
template<size_t N, typename T>
Rect<N, T> intersect(const Rect<N, T>&, const Rect<N, T>&);

// merge with a rect
template<size_t N, typename T>
Rect<N, T> merge(const Rect<N, T>&, const Rect<N, T>&);

// merge with a point
template<size_t N, typename T>
Rect<N, T> merge(const Rect<N, T>&, const Vector<N, T>&);

// test whether a point is inside
template<size_t N, typename T>
bool is_inside(const Rect<N, T>&, const Vector<N, T>&);

template<size_t N, typename T>
bool isnan(const Rect<N, T>&);

// returns as a vector
template<size_t N, typename T>
Vector<N*2, T> to_vector(const Rect<N, T>&);

#include <math/rect.inl>

NS_FLOW2D_MATH_END