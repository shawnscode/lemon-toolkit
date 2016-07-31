// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/defines.hpp>
#include <graphics/color.hpp>

NS_FLOW2D_GFX_BEGIN

struct Color
{
    Color() : r(1.f), g(1.f), b(1.f), a(1.f) {};
    Color(const Color&) = default;
    Color(const Color&, float);
    Color(std::initializer_list<float>);
    Color& operator = (const Color&) = default;

    // test for equality and equality with another color without epsilon.
    bool    operator == (const Color&) const;
    bool    operator != (const Color&) const;
    float   operator [] (size_t) const;

    // return the 'grayscale' representation of RGB values
    float grayscale() const;
    // return color packed to a 32-bit integer
    uint32_t to_uint32() const;

    // clip to [0, 1.0] range
    Color& clip();
    // inverts the RGB channels and optionally the alpha channel as well
    Color& invert(bool alpha = false);

    float r, g, b, a;

    static const Color WHITE;
    static const Color GRAY;
    static const Color BLACK;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color YELLOW;
    static const Color TRANSPARENT;
};

Color operator + (const Color&, const Color&);
Color operator - (const Color&, const Color&);
Color operator * (const Color&, const Color&);
Color operator * (const Color&, float);

// test for equality with epsilon
bool equals(const Color&, const Color&, float epsilon = std::numeric_limits<float>::epsilon());
// return linear interpolation o this color with another color
Color lerp(const Color&, const Color&, float t);

// INCLUDED METHODS OF COLOR
INLINE float Color::operator [] (size_t index) const
{
    ENSURE( index >= 0 && index < 4 );
    return *(&r + index);
}

INLINE bool Color::operator == (const Color& rhs) const
{
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

INLINE bool Color::operator != (const Color& rhs) const
{
    return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a;
}


INLINE float Color::grayscale() const
{
    return r * 0.299f + g * 0.587f + b * 0.114f;
}

INLINE uint32_t Color::to_uint32() const
{
    unsigned ur = (unsigned)std::max(std::min((int)(r*255.f), 255), 0);
    unsigned ug = (unsigned)std::max(std::min((int)(g*255.f), 255), 0);
    unsigned ub = (unsigned)std::max(std::min((int)(b*255.f), 255), 0);
    unsigned ua = (unsigned)std::max(std::min((int)(a*255.f), 255), 0);

    return ur << 24 | ug << 16 | ub << 8 | ua;
}

INLINE Color& Color::clip()
{
    r = (r>1.0f) ? 1.0f : ((r<0.f) ? 0.f : r);
    g = (g>1.0f) ? 1.0f : ((g<0.f) ? 0.f : g);
    b = (b>1.0f) ? 1.0f : ((b<0.f) ? 0.f : b);
    a = (a>1.0f) ? 1.0f : ((a<0.f) ? 0.f : a);

    return *this;
}

INLINE Color& Color::invert(bool alpha)
{
    clip();
    r = 1.f - r;
    g = 1.f - g;
    b = 1.f - b;
    if( alpha ) a = 1.f - a;

    return *this;
}

INLINE Color operator + (const Color& lhs, const Color& rhs)
{
    return Color { lhs.r+rhs.r, lhs.g+rhs.g, lhs.b+rhs.b, lhs.a+rhs.a };
}

INLINE Color operator - (const Color& lhs, const Color& rhs)
{
    return Color { lhs.r-rhs.r, lhs.g-rhs.g, lhs.b-rhs.b, lhs.a-rhs.a };
}

INLINE Color operator * (const Color& lhs, float t)
{
    return Color { lhs.r*t, lhs.g*t, lhs.b*t, lhs.a*t };
}

INLINE bool equals(const Color& lhs, const Color& rhs, float epsilon)
{
    return
        std::abs(lhs.r - rhs.r) < epsilon &&
        std::abs(lhs.g - rhs.g) < epsilon &&
        std::abs(lhs.b - rhs.b) < epsilon &&
        std::abs(lhs.a - rhs.a) < epsilon;
}

INLINE Color lerp(const Color& from, const Color& to, float t)
{
    if( t >= 1.f ) return to;
    if( t <= 0.f ) return from;
    return from + (to-from)*t;
}

NS_FLOW2D_GFX_END