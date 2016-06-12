// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <flow2d/math/vector.hpp>

NS_FLOW2D_BEGIN

// the coordinate space in which to operate
enum class TransformSpace : uint8_t
{
    SELF,
    WORLD
};

// an iterator over sub-transforms of the ancestor
struct TransformIterator : public std::iterator<std::input_iterator_tag, TransformComponent*>
{
    TransformIterator(TransformComponent* current) : cursor(current) {}

     TransformIterator&         operator ++ ();
     bool                       operator == (const TransformIterator&) const;
     bool                       operator != (const TransformIterator&) const;
     TransformComponent*        operator * ();
     const TransformComponent*  operator * () const;

 protected:
     TransformComponent* cursor = nullptr;
};

struct Transform
{
    Vector2f position;
    Vector2f scale;
    float    rotation;

    Transform(const Vector2f& position = kVector2fZero, const Vector2f& scale = kVector2fOne, float rotation = 0.f)
    : position(position), scale(scale), rotation(rotation)
    {}

    Transform(const Transform&) = default;
    Transform& operator = (const Transform&) = default;

    Transform operator * (const Transform&) const;
    Transform operator / (const Transform&) const;

    static const Transform IDENTITY;
};

// transform component is used to allow entities to be coordinated in the world.
// the transformation is Y = M*X+T, where M is a 2-by-2 matrix and T is a 1x2
// translation. in most case, M = R, a rotation matrix, or M = R*S, where
// R is a rotation matrix and S is a diagonal matrix whose disgonal entries
// are positives scales.
struct TransformComponent
{
    TransformComponent();
    TransformComponent(const Vector2f&, const Vector2f&, float);

    // non-copyable
    TransformComponent(const TransformComponent&) = delete;
    TransformComponent& operator = (const TransformComponent&) = delete;

    void identity();

    // get the matrix representation of transform in different space
    Transform   get_transform(TransformSpace space = TransformSpace::SELF) const;
    void        set_transform(const Transform&, TransformSpace space = TransformSpace::SELF);

    // setters and getters of transform properties
    void        set_scale(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void        set_position(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void        set_rotation(float, TransformSpace space = TransformSpace::SELF);

    Vector2f    get_scale(TransformSpace space = TransformSpace::SELF) const;
    float       get_rotation(TransformSpace space = TransformSpace::SELF) const;
    Vector2f    get_position(TransformSpace space = TransformSpace::SELF) const;

    //
    TransformComponent* set_parent(TransformComponent*);
    TransformComponent* get_parent();
    void                remove_from_parent();
    size_t              get_child_count() const;

    // iterators of this transform
    TransformIterator       begin();
    const TransformIterator begin() const;
    TransformIterator       end();
    const TransformIterator end() const;

protected:
    friend class TransformIterator;
    void update_children();

    Transform transform;
    Transform world_transform;

    TransformComponent*  parent = nullptr;
    TransformComponent*  first_child = nullptr;
    TransformComponent*  next_sibling = nullptr;
    TransformComponent*  prev_sibling = nullptr;
};

#include <flow2d/gameplay/transform.inl>
NS_FLOW2D_END