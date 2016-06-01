// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

template<typename T> struct Rect2
{
    Rect2();
    Rect2(std::initializer_list<T>);

    Rect2(const Rect2&) = default;
    Rect2& operator = (const Rect2&) = default;

    const Vector2<T>& operator[](int) const;
    Vector2<T>& operator[](int);

    bool operator == (const Rect2<T>&) const;
    bool operator != (const Rect2<T>&) const;

    Rect2<T>    operator + (const Rect2<T>&) const;
    Rect2<T>    operator + (const Vector2<T>&) const;
    Rect2<T>&   operator += (const Rect2<T>&);
    Rect2<T>&   operator += (const Vector2<T>&);

    bool equals(const Rect2& rh, T epslion = std::numeric_limits<T>::epsilon()) const;
    bool is_inside(const Vector2<T>&) const;

    T x() const;
    T y() const;
    T width() const;
    T height() const;

    Vector2<T> center() const;
    Vector2<T> size() const;
    Vector2<T> half_size() const;

    static const Rect2<T> FULL; // (-1, -1) ~ (1, 1)
    static const Rect2<T> POSITIVE; // (0, 0) ~ (1, 1)

protected:
    Vector2<T> m_min;
    Vector2<T> m_max;
};

using Rect2f = Rect2<float>;
using Rect2i = Rect2<int>;

#include <flow2d/math/rect.inl>

NS_FLOW2D_END