// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#include <math/color.hpp>

NS_LEMON_MATH_BEGIN

const Color Color::WHITE        = { 1.f, 1.f, 1.f, 1.f };
const Color Color::GRAY         = { 0.5f, 0.5f, 0.5f, 0.5f };
const Color Color::BLACK        = { 0.f, 0.f, 0.f, 1.f };
const Color Color::RED          = { 1.f, 0.f, 0.f, 1.f};
const Color Color::GREEN        = { 0.f, 1.f, 0.f, 1.f };
const Color Color::BLUE         = { 0.f, 0.f, 1.f, 1.f };
const Color Color::CYAN         = { 0.f, 1.f, 1.f, 1.f };
const Color Color::MAGENTA      = { 1.f, 0.f, 1.f, 1.f };
const Color Color::YELLOW       = { 1.f, 1.f, 0.f, 1.f };
const Color Color::TRANSPARENT  = { 0.f, 0.f, 0.f, 0.f };

Color::Color(std::initializer_list<float> values)
{
    auto size = values.size();
    auto cursor = values.begin();

    r = g = b = a = 1.0f;
    if( size >= 1 ) r = *cursor++;
    if( size >= 2 ) g = *cursor++;
    if( size >= 3 ) b = *cursor++;
    if( size >= 4 ) a = *cursor++;
}

Color::Color(uint32_t i)
{
    r = (float)((i & 0xFF000000) >> 24) / 255.f;
    g = (float)((i & 0xFF000000) >> 16) / 255.f;
    b = (float)((i & 0xFF000000) >>  8) / 255.f;
    a = (float)((i & 0xFF000000) >>  0) / 255.f;
}

Color::Color(const Color& rh, float a)
{
    *this = rh;
    a = a;
}

std::ostream& operator << (std::ostream& out, const Color& c)
{
    char buffer[32];
    sprintf(buffer, "0x%08X", c.to_uint32());
    out << "Color(" << buffer << ")";
    return out;
}

NS_LEMON_MATH_END