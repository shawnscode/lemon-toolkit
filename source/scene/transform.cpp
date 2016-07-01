// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/transform.hpp>
#include <scene/scene.hpp>

NS_FLOW2D_BEGIN

void Transform::visit_children(const visitor& cb, bool recursive)
{
    for( auto cursor = _first_child; cursor != nullptr; cursor = cursor->_next_sibling )
    {
        cb(*this, *cursor);
        if( recursive )
            cursor->visit_children(cb, recursive);
    }
}

void Transform::update_children()
{
    visit_children([](const Transform& parent, Transform& c)
    {
        c._worldspace = parent._worldspace * c._localspace;
    }, true);
}

void Transform::visit_ancestors(const visitor& cb)
{
    auto cursor = _parent;

    for( auto cursor = _parent; cursor != nullptr; cursor = cursor->_parent)
        cb(*this, *cursor);
}

void Transform::set_scale(const Vector2f& scale, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        _localspace.scale = scale;
        if( _parent )
            _worldspace = _parent->_worldspace * _localspace;
    }
    else
    {
        _worldspace.scale = scale;
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
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
    }
    else
    {
        _worldspace.position = position;
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
    }

    update_children();
}

void Transform::set_rotation(float rotation, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        _localspace.rotation = rotation;
        if( _parent )
            _worldspace = _parent->_worldspace * _localspace;
    }
    else
    {
        _worldspace.rotation = rotation;
        if( _parent )
            _localspace = _worldspace / _parent->_worldspace;
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

bool Transform::is_root() const
{
    return _parent == nullptr;
}

bool Transform::is_leaf() const
{
    return _first_child == nullptr;
}

Transform* Transform::get_parent()
{
    return _parent;
}

size_t Transform::get_children_count(bool recursive)
{
    size_t result = 0;
    visit_children([&](const Transform&, Transform&) { result ++; }, recursive);
    return result;
}

/// MEMBER METHODS
void Transform::on_spawn(EntityManager& world, Entity object) {}

void Transform::on_dispose(EntityManager& world, Entity object)
{
    remove_from_parent();
}

NS_FLOW2D_END