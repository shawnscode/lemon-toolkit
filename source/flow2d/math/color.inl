// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF COLOR

INLINE bool Color::operator == (const Color& rh) const
{
    return r == rh.r && g == rh.g && b == rh.b && a == rh.a;
}

INLINE bool Color::operator != (const Color& rh) const
{
    return r != rh.r || g != rh.g || b != rh.b || a != rh.a;
}

INLINE Color Color::operator + (const Color& rh) const
{
    return Color { r+rh.r, g+rh.g, b+rh.b, a+rh.a };
}

INLINE Color Color::operator - (const Color& rh) const
{
    return Color { r-rh.r, g-rh.g, b-rh.b, a-rh.a };
}

INLINE Color Color::operator * (float t) const
{
    return Color { r*t, g*t, b*t, a*t };
}

INLINE bool Color::equals(const Color& rh, float epsilon) const
{
    return
        std::abs(r - rh.r) < epsilon &&
        std::abs(g - rh.g) < epsilon &&
        std::abs(b - rh.b) < epsilon &&
        std::abs(a - rh.a) < epsilon;
}

INLINE float Color::grayscale() const
{
    return r * 0.299f + g * 0.587f + b * 0.114f;
}

INLINE Color Color::lerp(const Color& to, float t) const
{
    if( t >= 1.f ) return to;
    if( t <= 0.f ) return *this;
    return *this + (to-*this)*t;
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
