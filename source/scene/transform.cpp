// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/transform.hpp>

NS_FLOW2D_BEGIN

Transform::iterator::iterator(Transform* t, iterator_mode m)
: _mode(m)
{
    if( t != nullptr )
    {
        _start = t;
        _cusor = t->_first_child;
    }
    else
        _start = _cusor = nullptr;
}

Transform::view Transform::get_children(bool recursive)
{
    return view(this,
        recursive ? iterator_mode::CHILDREN_RECURSIVE : iterator_mode::CHILDREN);
}

Transform::view Transform::get_ancestors()
{
    return view(_parent, iterator_mode::ANCESTORS);
}

void Transform::update_children()
{
    get_children(true).visit([](Transform& t)
    {
        t._worldspace = t._parent->_worldspace * t._localspace;
    });
}

void Transform::set_scale(const Vector2f& scale, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        _localspace.scale = scale;
        if( _parent )
            _worldspace = _parent->_worldspace * _localspace;
        else
            _worldspace = _localspace;
    }
    else
    {
        _worldspace.scale = scale;
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
        else
            _localspace = _worldspace;
    }

    update_children();
}

void Transform::set_position(const Vector2f& position, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        _localspace.position = position;
        if( _parent )
            _worldspace = _parent->_worldspace * _localspace;
        else
            _worldspace = _localspace;
    }
    else
    {
        _worldspace.position = position;
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
        else
            _localspace = _worldspace;
    }

    update_children();
}

void Transform::set_rotation(float rotation, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        _localspace.rotation = math::degree_to_radians(rotation);
        if( _parent )
            _worldspace = _parent->_worldspace * _localspace;
        else
            _worldspace = _localspace;
    }
    else
    {
        _worldspace.rotation = math::degree_to_radians(rotation);
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
        else
            _localspace = _worldspace;
    }

    update_children();
}

Vector2f Transform::get_scale(TransformSpace space) const
{
    if( TransformSpace::SELF == space )
        return _localspace.scale;
    else
        return _worldspace.scale;
}

Vector2f Transform::get_position(TransformSpace space) const
{
    if( TransformSpace::SELF == space )
        return _localspace.position;
    else
        return _worldspace.position;
}

float Transform::get_rotation(TransformSpace space) const
{
    if( TransformSpace::SELF == space )
        return _localspace.rotation;
    else
        return _worldspace.rotation;
}

Vector2f Transform::transform_point(const Vector2f& point) const
{
    return point + _worldspace.position;
}

Vector2f Transform::inverse_transform_point(const Vector2f& point) const
{
    return point - _worldspace.position;
}

Vector2f Transform::transform_vector(const Vector2f& v) const
{
    auto sn = std::sin(_worldspace.rotation);
    auto cs = std::cos(_worldspace.rotation);

    return Vector2f { v[0]*cs - v[1]*sn, v[0]*sn + v[1]*cs } * _worldspace.scale;
}

Vector2f Transform::inverse_transform_vector(const Vector2f& v) const
{
    auto sn = std::sin(-_worldspace.rotation);
    auto cs = std::cos(-_worldspace.rotation);

    return Vector2f { v[0]*cs - v[1]*sn, v[0]*sn + v[1]*cs } / _worldspace.scale;
}

Vector2f Transform::transform_direction(const Vector2f& d) const
{
    auto sn = std::sin(_worldspace.rotation);
    auto cs = std::cos(_worldspace.rotation);

    return Vector2f { d[0]*cs - d[1]*sn, d[0]*sn + d[1]*cs };
}

Vector2f Transform::inverse_transform_direction(const Vector2f& d) const
{
    auto sn = std::sin(-_worldspace.rotation);
    auto cs = std::cos(-_worldspace.rotation);

    return Vector2f { d[0]*cs - d[1]*sn, d[0]*sn + d[1]*cs };
}

void Transform::append_child(Transform& transform, bool keep_world_pose)
{
    if( transform._parent != nullptr )
        transform.remove_from_parent();

    if( _first_child != nullptr )
    {
        ENSURE( _first_child->_prev_sibling == nullptr );
        _first_child->_prev_sibling = &transform;
        transform._next_sibling = _first_child;
    }

    _first_child = &transform;
    transform._parent = this;

    if( !keep_world_pose )
        transform._worldspace = _worldspace * transform._localspace;
    else
        transform._localspace = transform._worldspace / _worldspace;

    transform.update_children();
}

void Transform::remove_from_parent()
{
    if( _parent == nullptr )
        return;

    if( _parent->_first_child == this )
    {
        _parent->_first_child = _next_sibling;
        if( _next_sibling )
            _next_sibling->_prev_sibling = nullptr;
    }
    else
    {
        ENSURE( _prev_sibling != nullptr );
        _prev_sibling->_next_sibling = _next_sibling;
        if( _next_sibling )
            _next_sibling->_prev_sibling = _prev_sibling;
    }

    _parent = nullptr;
    _prev_sibling = nullptr;
    _next_sibling = nullptr;
}

/// MEMBER METHODS
void Transform::on_spawn(EntityManager& world, Entity object)
{
    _world = &world;
    _object = object;
}

void Transform::on_dispose(EntityManager& world, Entity object)
{
    remove_from_parent();
}

NS_FLOW2D_END