// @date 2016/07/19
// @author Mao Jingkai(oammix@gmail.com)

INLINE float degree_to_radians(float degree)
{
    return (degree%360) / 180 * pi;
}

INLINE float radians_to_degree(float radians)
{
    if( radians > pi*2 )

    return radians * 180 / pi;
}
