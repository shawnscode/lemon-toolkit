#pragma once

#include "flow2d.hpp"
#include "glm/vec2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

NS_FLOW2D_BEGIN

struct TransformName
{
    char[32]    name;

    TransformName() { name[0] = '\0'; }
    void reset(const char*);
    bool operator == (const char*) const;
    bool operator == (const TransformName&) const;
};

// the coordinate space in which to operate
enum class TransformSpace : uint8_t
{
    SELF,
    WORLD
};

// an iterator over sub-transforms of ancester
struct TransformIterator : public std::iterator<std::input_iterator_tag, Transform*>
{
    TransformIterator(Transform*);

    TransformIterator&  operator ++ ();
    bool                operator == (const TransformIterator&) const;
    bool                operator != (const TransformIterator&) const;
    Transform*          operator * ();
    const Transform*    operator * () const;
};

// transform component is used to allow entities to be positioned and coordinated in the world
struct Transform
{
    Transform(Transform*);
    Transform(Transform*, const glm::vec2&);
    Transform(Transform*, const glm::vec2&, const glm::vec2&, float);

    // non-copyable
    Transform() = default;
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    // setters and getters of transform properties
    void set_scale(const glm::vec2&, TransformSpace space = TransformSpace::SELF);
    void set_position(const glm::vec2&, TransformSpace space = TransformSpace::SELF);
    void set_rotation(float, TransformSpace space = TransformSpace::SELF);

    glm::vec2   get_scale(TransformSpace space = TransformSpace::SELF) const;
    glm::vec2   get_position(TransformSpace space = TransformSpace::SELF) const;
    float       get_rotation(TransformSpace space = TransformSpace::SELF) const;

    // moves the transform in the direction and distance of translation
    void translate(const glm::vec2&, TransformSpace space = TransformSpace::SELF);
    // applies a rotation
    void rotate(float, TransformSpace space = TransformSpace::SELF);
    // rotates the transform about axis passing through point in world coordinates by angle degrees.
    void rotate_around(float, const glm::vec2&, TransformSpace space = TransformSpace::SELF);
    // rotates the transform so the forward vector points at /target/'s current position
    void look_at(const glm::vec2&, TransformSpace space = TransformSpace::SELF);
    // get the matrix representation of current transform
    const glm::mat3& get_transform(TransformSpace space = TransformSpace::SELF) const;

    // transforms position from local space to world space
    glm::vec2 transform_point(const glm::vec2&) const;
    // same as above, except not affected by position of the transform
    glm::vec2 transform_vector(const glm::vec2&) const;
    // same as above, except not affected by position and scale of the transform
    glm::vec2 transform_direction(const glm::vec2&) const;

    //
    Transform*  set_parent(Transform*);
    Transform*  get_parent();
    size_t      get_child_count() const;

    TransformIterator begin();
    TransformIterator end();
    const TransformIterator begin() const;
    const TransformIterator end() const;

    // finds a child by name and returns it
    // if name contains a '/' character it will traverse the hierarchy like a path name
    void set_name(const char*);
    const char* get_name() const;
    Transform* find_with_name(const char*);

protected:
    // friend class SceneGraphSystem;
    // friend class ActionSystem;

    glm::vec2   m_position;
    glm::vec2   m_scale;
    float       m_rotation;
    glm::mat3   m_world_transform;

    TransformName   m_name;
    Transform*      m_parent = nullptr;
    Transform*      m_first_child = nullptr;
    Transform*      m_next_sibling = nullptr;
    Transform*      m_prev_sibling = nullptr;
};

#include "transform.inl"
NS_FLOW2D_END