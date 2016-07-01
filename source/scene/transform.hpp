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
struct Transform : public Component
{
    // setters and getters of transform properties
    static void set_scale(EntityManager&, Transform&, const Vector2f&, TransformSpace space = TransformSpace::SELF);
    static void set_position(EntityManager&, Transform&, const Vector2f&, TransformSpace space = TransformSpace::SELF);
    static void set_rotation(EntityManager&, Transform&, float, TransformSpace space = TransformSpace::SELF);

    static Vector2f get_scale(EntityManager&, const Transform&, TransformSpace space = TransformSpace::SELF);
    static Vector2f get_position(EntityManager&, const Transform&, TransformSpace space = TransformSpace::SELF);
    static float    get_rotation(EntityManager&, const Transform&, TransformSpace space = TransformSpace::SELF);

    // visit all of this components' ancestors,
    // in depth-first order if works with recursive mode.
    using visitor = std::function<void(const Transform&, Transform&)>;
    static void visit_children(EntityManager&, Transform&, const visitor&, bool recursive = false);
    static void update_children(EntityManager&, Transform&);

    // appends an entity to this hierarchy
    static void append_child(EntityManager&, Transform&, Transform&, bool keep_world_pose = false);
    // remove this branch from its parent hierarchy
    static void remove_from_parent(EntityManager&, Transform&);
    // returns true if this is the root of a hierarchy, aka. has no parent
    static bool is_root(EntityManager&, const Transform&);
    // returns true if this is the leaf of a hierarchy, aka. has no children
    static bool is_leaf(EntityManager&, const Transform&);
    // returns parent entity
    static Entity get_parent(EntityManager&, const Transform&);
    // returns the number of direct _children in this hierarchy
    static size_t get_children_count(EntityManager&, const Transform&, bool recursive = false);

    Transform() = default;
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    Transform(const Vector2f& position, const Vector2f& scale = {1.0f, 1.0f}, float rotation = 0.f)
    : _localspace(position, scale, rotation), _worldspace(position, scale, rotation)
    {}

    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

protected:
    TransformMatrix _localspace;
    TransformMatrix _worldspace;

    Entity      _handle;
    Transform*  _parent         = nullptr;
    Transform*  _first_child    = nullptr;
    Transform*  _next_sibling   = nullptr;
    Transform*  _prev_sibling   = nullptr;
};

NS_FLOW2D_END