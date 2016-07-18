// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <math/vector.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<size_t N, typename T> struct Rect
{
    Rect();
    Rect(const Vector<N, T>&, const Vector<N, T>& extent = {0.f, 0.f});

    Rect(const Rect&) = default;
    Rect& operator = (const Rect&) = default;

    bool operator == (const Rect<N, T>&) const;
    bool operator != (const Rect<N, T>&) const;

    Rect<N, T>  operator +  (const Rect<N, T>&) const;
    Rect<N, T>  operator +  (const Vector<N, T>&) const;
    Rect<N, T>& operator += (const Rect<N, T>&);
    Rect<N, T>& operator += (const Vector<N, T>&);

    bool is_inside(const Vector<N, T>&) const;

    Vector<N, T> position() const;
    Vector<N, T> center() const;
    Vector<N, T> size() const;
    T            area() const;

    template<size_t Axis> T length() const;
    template<size_t Axis> T lower() const;
    template<size_t Axis> T upper() const;

protected:
    Vector<N, T> m_position, m_corner;
};

template<typename T>
using Rect2  = Rect<2, T>;
using Rect2f = Rect<2, float>;
using Rect2i = Rect<2, int>;

template<size_t N, typename T>
bool equals(const Rect<N, T>&, const Rect<N, T>&, T epslion = std::numeric_limits<T>::epsilon());

template<size_t N, typename T>
Rect<N, T> intersect(const Rect<N, T>&, const Rect<N, T>&);

#include <math/rect.inl>

NS_FLOW2D_END