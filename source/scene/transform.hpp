// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <math/vector.hpp>

NS_FLOW2D_BEGIN

// the coordinate space in which to operate
enum class TransformSpace : uint8_t
{
    SELF,
    WORLD
};

struct TransformMatrix
{
    Vector2f position;
    Vector2f scale;
    float    rotation;

    TransformMatrix(const Vector2f& position = {0.f, 0.f}, const Vector2f& scale = {1.f, 1.f}, float rotation = 0.f)
    : position(position), scale(scale), rotation(rotation)
    {}

    TransformMatrix(const TransformMatrix&) = default;
    TransformMatrix& operator = (const TransformMatrix&) = default;

    INLINE TransformMatrix operator * (const TransformMatrix& rh) const
    {
        return TransformMatrix(
            position + rh.position,
            { scale[0] * rh.scale[0], scale[1] * rh.scale[1] },
            rotation + rh.rotation);
    }

    INLINE TransformMatrix operator / (const TransformMatrix& rh) const
    {
        return TransformMatrix(
            position - rh.position,
            { scale[0] / rh.scale[0], scale[1] / rh.scale[1] },
            rotation - rh.rotation);
    }

    static const TransformMatrix IDENTITY;
};

// transform component is used to allow entities to be coordinated in the world.
struct Transform : public Component<Transform>
{
    Transform() = default;
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    Transform(const Vector2f& position, const Vector2f& scale = {1.0f, 1.0f}, float rotation = 0.f)
    : _localspace(position, scale, rotation), _worldspace(position, scale, rotation)
    {}

    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

protected:
    friend class NSScene;

    TransformMatrix _localspace;
    TransformMatrix _worldspace;

    Entity      _object;
    Transform*  _parent         = nullptr;
    Transform*  _first_child    = nullptr;
    Transform*  _next_sibling   = nullptr;
    Transform*  _prev_sibling   = nullptr;
};

NS_FLOW2D_END