// @date 2016/07/19
// @author Mao Jingkai(oammix@gmail.com)

INLINE float degree_to_radians(float degree)
{
    return std::fmod(degree, 360.f) / 180 * pi;
}

INLINE float radians_to_degree(float radians)
{
    return std::fmod(radians, pi*2) * 180 / pi;
}
