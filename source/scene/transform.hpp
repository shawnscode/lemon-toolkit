// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/ecs.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>

NS_LEMON_BEGIN

using namespace math;

// the coordinate space in which to operate
enum class TransformSpace : uint8_t
{
    LOCAL = 0,
    WORLD
};

struct TransformPose
{
    Vector3f position;
    Vector3f scale;
    Quaternion rotation;

    TransformPose(
        const Vector3f& position = {0.f, 0.f, 0.f},
        const Vector3f& scale = {1.f, 1.f, 1.f},
        const Quaternion& rotation = Quaternion(1.0f, 0.f, 0.f, 0.f))
    : position(position), scale(scale), rotation(rotation)
    {}
};

TransformPose operator * (const TransformPose&, const TransformPose&);
TransformPose operator / (const TransformPose&, const TransformPose&);

TransformPose& operator *= (TransformPose&, const TransformPose&);
TransformPose& operator /= (TransformPose&, const TransformPose&);

// transform component is used to allow entities to be coordinated in the world
struct Transform : public core::Component
{
protected:
    template<typename T> using it_function = std::function<T*(T*)>;
    template<typename T> struct iterator_traits : public std::iterator<std::forward_iterator_tag, T*>
    {
        iterator_traits(T*, const it_function<T>&, core::Component::Mask);

        iterator_traits& operator ++ ();
        iterator_traits operator ++ (int);
        bool operator == (const iterator_traits&) const;
        bool operator != (const iterator_traits&) const;
        T* operator * ();

    protected:
        T* _current = nullptr;
        it_function<T> _iterator;
        core::Component::Mask _mask;
    };

    using iterator = iterator_traits<Transform>;
    using const_iterator = iterator_traits<const Transform>;

    template<typename T, typename ... Args> struct view_traits
    {
        view_traits(T*, const it_function<T>&);

        iterator_traits<T> begin() const;
        iterator_traits<T> end() const;

        // visit the *iterator in sequences
        void visit(const std::function<void(Transform&, Args&...)>&);
        // returns the number of children in this hierachy
        unsigned count() const;

    protected:
        T* _transform = nullptr;
        it_function<T> _iterator;
        core::Component::Mask _mask;
    };

    template<typename ... Args> using view = view_traits<Transform, Args...>;
    template<typename ... Args> using const_view = view_traits<const Transform, Args...>;

public:
    Transform() = default;
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    Transform(core::Entity& entity,
        const Vector3f& position = {0.f, 0.f, 0.f},
        const Vector3f& scale = {1.f, 1.f, 1.f},
        const Quaternion& rotation = Quaternion(1.f, 0.f, 0.f, 0.f))
    : entity(entity), _pose(position, scale, rotation), _world_pose(position, scale, rotation)
    {}

    void dispose() { remove_from_parent(); }

    bool operator == (const Transform&) const;
    bool operator != (const Transform&) const;

    // setters and getters of transform properties in parents or world space
    void set_scale(const Vector3f&, TransformSpace space = TransformSpace::LOCAL);
    void set_position(const Vector3f&, TransformSpace space = TransformSpace::LOCAL);
    void set_rotation(const Vector3f&, TransformSpace space = TransformSpace::LOCAL);
    void set_rotation(const Quaternion&, TransformSpace space = TransformSpace::LOCAL);

    void scale(const Vector3f&);
    void rotate(const Quaternion&);
    void translate(const Vector3f&);

    Vector3f get_scale(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_position(TransformSpace space = TransformSpace::LOCAL) const;
    Quaternion get_rotation(TransformSpace space = TransformSpace::LOCAL) const;

    // returns direction
    Vector3f get_forward(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_back(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_up(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_down(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_right(TransformSpace space = TransformSpace::LOCAL) const;
    Vector3f get_left(TransformSpace space = TransformSpace::LOCAL) const;

    // transforms the position from local space to world space
    Vector3f transform_point(const Vector3f&) const;
    // the opposite of Transform::transform_point
    Vector3f inverse_transform_point(const Vector3f&) const;
    // transforms the vector from local space to world space,
    // this operation is not affected by position of the transform, but is is affected by scale and rotation.
    Vector3f transform_vector(const Vector3f&) const;
    // ths opposite of Transform::transform_vector
    Vector3f inverse_transform_vector(const Vector3f&) const;
    // transforms direction from local space to world space,
    // this operation is not affected by scale or position of the transform.
    Vector3f transform_direction(const Vector3f&) const;
    // ths opposite of Transform::transform_direction
    Vector3f inverse_transform_direction(const Vector3f&) const;

    // visit all of this object's ancestors/decenster,
    // in depth-first order if works with recursive mode.
    view<> find_ancestors();
    const_view<> find_ancestors() const;
    view<> find_children(bool recursive = false);
    const_view<> find_children(bool recursive = false) const;
    // find children that have all of the specified components with deepth first search
    template<typename ... T> view<T...> find_children_with(bool recursive = false);
    template<typename ... T> const_view<T...> find_children_with(bool recursive = false) const;

    // appends an entity to this hierarchy
    void append_child(Transform&, bool keep_world_pose = false);
    // remove this branch from its parent hierarchy
    void remove_from_parent();
    // returns true if this is the root of a hierarchy, aka. has no parent
    bool is_root() const;
    // returns true if this is the leaf of a hierarchy, aka. has no children
    bool is_leaf() const;
    // returns true if target is one of the ancestor of this transfrom
    bool is_ancestor(Transform&) const;
    // returns parent entity
    Transform* get_parent();
    const Transform* get_parent() const;
    // returns root entity
    Transform* get_root();
    const Transform* get_root() const;

    // returns representation of matrix
    Matrix4f get_model_matrix(TransformSpace space = TransformSpace::LOCAL) const;
    Matrix3f get_normal_matrix(TransformSpace space = TransformSpace::LOCAL) const;

protected:
    template<typename T> static T* find_parent(T* current);
    template<typename T> static T* find_next_children(T* start, T* current);
    template<typename T> static T* find_next_children_recursive(T* start, T* current);
    template<typename T> static T* find_with_mask(const it_function<T>&, T*, core::Component::Mask);

public:
    core::Entity& entity;

protected:
    // update the world pose of children
    void update_children();

    TransformPose _pose;
    TransformPose _world_pose;

    Transform* _parent = nullptr;
    Transform* _first_child = nullptr;
    Transform* _next_sibling = nullptr;
    Transform* _prev_sibling = nullptr;
};

#include <scene/transform.inl>
NS_LEMON_END
