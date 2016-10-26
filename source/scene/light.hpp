// @date 2016/10/19
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/ecs.hpp>
#include <math/color.hpp>
#include <math/vector.hpp>

NS_LEMON_BEGIN

struct Transform;
struct Light
{
    // the intensity of light is multiplied with the light color,
    // the value will be clamped between 0.f and 1.f
    void set_intensity(float);
    // the color of the light
    void set_color(const math::Color&);

    float get_intensity() const;
    const math::Color& get_color() const;

    // used to evaluate the influent strength on target position
    virtual float evaluate(const Transform&, const Transform&) const = 0;

protected:
    float _intensity = 1.0f;
    math::Color _color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct DirectionalLight : public core::Component, Light
{
    float evaluate(const Transform& self, const Transform& rhs) const override;
};

struct PointLight : public core::Component, Light
{
    float evaluate(const Transform& self, const Transform& rhs) const override;

    // set the attenuation factors
    void set_attenuation_factors(const math::Vector3f&);
    const math::Vector3f& get_attenuation_factors() const;

protected:
    math::Vector3f _attenuations = { 1.0f, 0.09f, 0.032f };
};

struct SpotLight : public core::Component, Light
{
    float evaluate(const Transform&, const Transform& rhs) const override;

    // set the cutoff angle of the spot cone in degrees
    void set_cutoff(float);
    // set the range of the light
    void set_range(float);

    float get_cutoff() const;
    float get_range() const;

protected:
    float _cutoff = 45.f;
    float _range = 10.f;
};

INLINE void Light::set_intensity(float i)
{
    _intensity = math::clamp(i, 0.f, 1.0f);
}

INLINE void Light::set_color(const math::Color& color)
{
    _color = color;
}

INLINE float Light::get_intensity() const
{
    return _intensity;
}

INLINE const math::Color& Light::get_color() const
{
    return _color;
}

INLINE void PointLight::set_attenuation_factors(const math::Vector3f& v)
{
    _attenuations = v;
}

INLINE const math::Vector3f& PointLight::get_attenuation_factors() const
{
    return _attenuations;
}

INLINE void SpotLight::set_cutoff(float v)
{
    _cutoff = v;
}

INLINE float SpotLight::get_cutoff() const
{
    return _cutoff;
}

INLINE void SpotLight::set_range(float v)
{
    _range = v;
}

INLINE float SpotLight::get_range() const
{
    return _range;
}

NS_LEMON_END
