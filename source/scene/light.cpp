// @date 2016/10/19
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/light.hpp>
#include <scene/transform.hpp>

NS_LEMON_BEGIN

float DirectionalLight::evaluate(const Transform& self, const Transform& rhs) const
{
    return _intensity;
}

float PointLight::evaluate(const Transform& self, const Transform& rhs) const
{
    return _intensity;
//    float distance = distance(
//        self.get_position(TransformSpace::WORLD),
//        rhs.get_position(TransformSpace::WORLD));
//    return _intensity / (_constant + _linear*distance + _quadratic*(distance*distance));
}

float SpotLight::evaluate(const Transform& self, const Transform& rhs) const
{
//    float distance = distance(
//        self.get_position(TransformSpace::WORLD),
//        rhs.get_position(TransformSpace::WORLD));
//    return _intensity * (distance > _range ? 0.f : 1.0f);
    return  _intensity;
}

NS_LEMON_END
