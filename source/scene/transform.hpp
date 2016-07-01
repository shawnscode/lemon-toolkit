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

    // setters and getters of transform properties
    void set_scale(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void set_position(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void set_rotation(float, TransformSpace space = TransformSpace::SELF);

    Vector2f get_scale(TransformSpace space = TransformSpace::SELF) const;
    Vector2f get_position(TransformSpace space = TransformSpace::SELF) const;
    float    get_rotation(TransformSpace space = TransformSpace::SELF) const;

    // visit all of this components' children,
    // in depth-first order if works with recursive mode.
    using visitor = std::function<void(const Transform&, Transform&)>;
    void visit_children(const visitor&, bool recursive = false);
    void update_children();
    // visit all of this components' ancestors
    void visit_ancestors(const visitor&);

    // appends an entity to this hierarchy
    void append_child(Transform&, bool keep_world_pose = false);
    // remove this branch from its parent hierarchy
    void remove_from_parent();
    // returns true if this is the root of a hierarchy, aka. has no parent
    bool is_root() const;
    // returns true if this is the leaf of a hierarchy, aka. has no children
    bool is_leaf() const;
    // returns parent entity
    Transform* get_parent();
    // returns the number of direct _children in this hierarchy
    size_t get_children_count(bool recursive = false); // const

protected:
    TransformMatrix _localspace;
    TransformMatrix _worldspace;

    Transform*  _parent         = nullptr;
    Transform*  _first_child    = nullptr;
    Transform*  _next_sibling   = nullptr;
    Transform*  _prev_sibling   = nullptr;
};

NS_FLOW2D_END