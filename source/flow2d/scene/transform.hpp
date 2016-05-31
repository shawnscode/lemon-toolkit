// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>

#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>

NS_FLOW2D_BEGIN

// the coordinate space in which to operate
enum class TransformSpace : uint8_t
{
    SELF,
    WORLD
};

// an iterator over sub-transforms of the ancestor
struct TransformIterator : public std::iterator<std::input_iterator_tag, Transform*>
{
    TransformIterator(Transform* current) : m_cursor(current) {}

     TransformIterator&  operator ++ ();
     bool                operator == (const TransformIterator&) const;
     bool                operator != (const TransformIterator&) const;
     Transform*          operator * ();
     const Transform*    operator * () const;

 protected:
     Transform* m_cursor = nullptr;
};

// transform component is used to allow entities to be coordinated in the world.
// the transformation is Y = M*X+T, where M is a 2-by-2 matrix and T is a 1x2
// translation. in most case, M = R, a rotation matrix, or M = R*S, where
// R is a rotation matrix and S is a diagonal matrix whose disgonal entries
// are positives scales.
struct Transform
{
    Transform();
    Transform(Transform*);
    Transform(Transform*, const glm::vec2&, const glm::vec2&, float);

    // non-copyable
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    // setters and getters of transform properties
    void reset();
    void set_scale(const glm::vec2&);
    void set_rotation(float);
    void set_position(const glm::vec2&);

    glm::vec2   get_scale() const;
    float       get_rotation() const;
    glm::vec2   get_position() const;

    // moves the transform in the direction and distance of translation
    void translate(const glm::vec2&);
    // applies a rotation in degree
    void rotate(float);
    // get the matrix representation of transform in different space
    const glm::mat3& get_transform() const;

    //
    Transform*  set_parent(Transform*);
    Transform*  get_parent();
    size_t      get_child_count() const;

    // finds a child by name and returns it
    // if name contains a '/' character it will traverse the hierarchy like a path name
    void        set_name(const char*);
    const char* get_name() const;
    Transform*  find_with_name(const char*);

    // iterators of this transform
    TransformIterator       begin();
    const TransformIterator begin() const;
    TransformIterator       end();
    const TransformIterator end() const;

protected:
    friend class TransformIterator;
    friend class SceneSystem;

    void set_scene(SceneSystem*);
    void detach_from_parent();

    glm::vec2   m_position;
    glm::vec2   m_scale;
    float       m_rotation;

    mutable bool        m_update_transform;
    mutable glm::mat3   m_transform;
    mutable bool        m_update_decents;
    mutable glm::mat3   m_transform_to_world;

    std::string     m_name;
    SceneSystem*    m_scene = nullptr;
    Transform*      m_parent = nullptr;
    Transform*      m_first_child = nullptr;
    Transform*      m_next_sibling = nullptr;
    Transform*      m_prev_sibling = nullptr;
};

#include <flow2d/scene/transform.inl>
NS_FLOW2D_END