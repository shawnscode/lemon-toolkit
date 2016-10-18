// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/ecs.hpp>
#include <math/matrix.hpp>
#include <math/color.hpp>

#include <graphics/graphics.hpp>

NS_LEMON_BEGIN

using CameraLayerMask = std::bitset<kMaxRenderLayer>;

// a camera is a device through which the player views the world.
struct Transform;
struct Camera
{
    static math::Matrix4f get_view_matrix(Transform&);

public:
    // set near/far clipping plane distance
    void set_clip_plane(float near, float far);
    // set cull mask used to render parts of the scene selectively
    void set_cull_mask(CameraLayerMask);
    // set the aspect ratio (width divided by height)
    void set_aspect(float);
    // set how the camera clears the background
    void set_clear(graphics::ClearOption, const math::Color&, float depth = 1.f, unsigned stencil = 0);

    // returns the near clipping plane distance
    float get_near_clip() const;
    // returns the far clipping plane distance
    float get_far_clip() const;
    // returns the layer cull mask
    CameraLayerMask get_cull_mask() const;
    // returns matrix
    virtual math::Matrix4f get_projection_matrix() const = 0;

protected:
    float _aspect = 1.f;
    float _near_clip = 0.1f;
    float _far_clip = 100.f;
    CameraLayerMask _cull_mask;

    graphics::ClearOption _clear_option;
    math::Color _clear_color;   
    float _clear_depth;
    float _clear_stencil;
};

struct OrthoCamera : public Camera, core::Component
{
    // set the camera's vertical half-size
    void set_orthographic_size(float);
    // returns the camera's vertical half-size
    float get_orthographic_size() const;
    // returns projection matrix
    math::Matrix4f get_projection_matrix() const override;

protected:
    float _orthographic_size = 1.f;
};

struct PerspectiveCamera : public Camera, core::Component
{
    // set the field of view of the camera in degrees
    void set_field_of_view(float);
    // returns the field of view
    float get_field_of_view() const;
    // returns projection matrix
    math::Matrix4f get_projection_matrix() const override;

protected:
    float _field_of_view = 45.f;
};

//
// IMPLEMENTATIONS of CAMERA
INLINE void Camera::set_clip_plane(float near, float far)
{
    _near_clip = near;
    _far_clip = far;
}

INLINE void Camera::set_cull_mask(CameraLayerMask mask)
{
    _cull_mask = mask;
}

INLINE void Camera::set_aspect(float aspect)
{
    _aspect = aspect;
}

INLINE void Camera::set_clear(graphics::ClearOption options, const math::Color& color, float depth, unsigned stencil)
{
    _clear_option = options;
    _clear_color = color;
    _clear_depth = depth;
    _clear_stencil = stencil;
}

INLINE float Camera::get_near_clip() const
{
    return _near_clip;
}

INLINE float Camera::get_far_clip() const
{
    return _far_clip;
}

INLINE CameraLayerMask Camera::get_cull_mask() const
{
    return _cull_mask;
}

INLINE void OrthoCamera::set_orthographic_size(float size)
{
    _orthographic_size = size;
}

INLINE float OrthoCamera::get_orthographic_size() const
{
    return _orthographic_size;
}

INLINE math::Matrix4f OrthoCamera::get_projection_matrix() const
{
    auto hsize = _orthographic_size * _aspect;
    return math::ortho(-hsize, hsize, -_orthographic_size, _orthographic_size, _near_clip, _far_clip);
}

INLINE void PerspectiveCamera::set_field_of_view(float fov)
{
    _field_of_view = fov;
}

INLINE float PerspectiveCamera::get_field_of_view() const
{
    return _field_of_view;
}

INLINE math::Matrix4f PerspectiveCamera::get_projection_matrix() const
{
    return math::perspective(_field_of_view, _aspect, _near_clip, _far_clip);
}


NS_LEMON_END