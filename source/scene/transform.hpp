// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

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
struct Transform : public Component<>
{
protected:
    enum class iterator_mode : uint8_t
    {
        ANCESTORS,
        CHILDREN,
        CHILDREN_RECURSIVE
    };

    struct iterator : public std::iterator<std::forward_iterator_tag, int>
    {
        iterator(Transform* t, iterator_mode m);

        iterator&   operator ++ ();
        bool        operator == (const iterator&) const;
        bool        operator != (const iterator&) const;
        Transform&  operator * () const;

    protected:
        Transform*      _cusor, *_start;
        iterator_mode   _mode;
    };

    struct view
    {
        view(Transform* t, iterator_mode m) : _start(t), _mode(m) {}

        iterator begin() const;
        iterator end() const;

        using visitor = std::function<void(Transform&)>;
        void visit(const visitor&);
        // returns the number of direct _children in this hierarchy
        virtual size_t count() const;

    protected:
        Transform*      _start;
        iterator_mode   _mode;
    };

    template<typename ... T> struct view_trait : public view
    {
        view_trait(Transform* t, iterator_mode m)
        : view(t, m), _mask(t->_world->get_components_mask<T...>()) {};

        using visitor = std::function<void(Transform&, T& ...)>;
        void visit(const visitor&);
        virtual size_t count() const override;

    protected:
        ComponentMask _mask;
    };

public:
    Transform() = default;
    Transform(const Transform&) = delete;
    Transform& operator = (const Transform&) = delete;

    Transform(const Vector2f& position, const Vector2f& scale = {1.0f, 1.0f}, float rotation = 0.f)
    : _localspace(position, scale, rotation), _worldspace(position, scale, rotation)
    {}

    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

    // setters and getters of transform properties in parents or world space
    void set_scale(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void set_position(const Vector2f&, TransformSpace space = TransformSpace::SELF);
    void set_rotation(float, TransformSpace space = TransformSpace::SELF);

    Vector2f get_scale(TransformSpace space = TransformSpace::SELF) const;
    Vector2f get_position(TransformSpace space = TransformSpace::SELF) const;
    float    get_rotation(TransformSpace space = TransformSpace::SELF) const;

    // update the world pose of children
    void update_children();

    // visit all of this object's ancestors/decenster,
    // in depth-first order if works with recursive mode.
    view get_ancestors();
    view get_children(bool recursive = false);
    // find children that have all of the specified components
    template<typename ... T> view_trait<T...> get_children_with(bool recursive = false);

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
    // returns representation of matrix
    Matrix3f to_matrix(TransformSpace space = TransformSpace::SELF) const;

    ////
    Entity get_object() const;
    template<typename T, typename ... Args> T* add_component(Args && ... args);
    template<typename T> T* get_component();
    template<typename T> void remove_component();
    template<typename T> bool has_component() const;

protected:
    Entity          _object;
    EntityManager*  _world = nullptr;

    TransformMatrix _localspace;
    TransformMatrix _worldspace;

    Transform*  _parent         = nullptr;
    Transform*  _first_child    = nullptr;
    Transform*  _next_sibling   = nullptr;
    Transform*  _prev_sibling   = nullptr;
};

#include <scene/transform.inl>
NS_FLOW2D_END