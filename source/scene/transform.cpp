// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/transform.hpp>
#include <functional>

NS_LEMON_BEGIN

Transform::view<> Transform::find_children(bool recursive)
{
    return view<>(this, std::bind(recursive ?
        &Transform::find_next_children_recursive<Transform> : &Transform::find_next_children<Transform>,
        this,
        std::placeholders::_1));
}

Transform::const_view<> Transform::find_children(bool recursive) const
{
   return const_view<>(this, std::bind(recursive ? 
        &Transform::find_next_children_recursive<const Transform> : &Transform::find_next_children<const Transform>,
        this,
        std::placeholders::_1));
}

Transform::view<> Transform::find_ancestors()
{
   return view<>(this, &Transform::find_parent<Transform>);
}

Transform::const_view<> Transform::find_ancestors() const
{
   return const_view<>(this, &Transform::find_parent<const Transform>);
}

bool Transform::is_ancestor(Transform& target) const
{
    for( auto ancestor : find_ancestors() )
        if( *ancestor == target )
            return true;
    return false;
}

void Transform::update_children()
{
    find_children(true).visit([](Transform& t)
    {
        t._world_pose = t._parent->_world_pose * t._pose;
    });
}

void Transform::set_scale(const Vector3f& scale, TransformSpace space)
{
    if( TransformSpace::LOCAL == space )
    {
        _pose.scale = scale;
        if( _parent )
            _world_pose = _parent->_world_pose * _pose;
        else
            _world_pose = _pose;
    }
    else
    {
        _world_pose.scale = scale;
        if( _parent )
            _pose = _world_pose / _parent->_world_pose;
        else
            _pose = _world_pose;
    }

    update_children();
}

void Transform::set_position(const Vector3f& position, TransformSpace space)
{
    if( TransformSpace::LOCAL == space )
    {
        _pose.position = position;
        if( _parent )
            _world_pose = _parent->_world_pose * _pose;
        else
            _world_pose = _pose;
    }
    else
    {
        _world_pose.position = position;
        if( _parent )
            _pose = _world_pose / _parent->_world_pose;
        else
            _pose = _world_pose;
    }

    update_children();
}

void Transform::set_rotation(const Quaternion& rotation, TransformSpace space)
{
    if( TransformSpace::LOCAL == space )
    {
        _pose.rotation = rotation;
        if( _parent )
            _world_pose = _parent->_world_pose * _pose;
        else
            _world_pose = _pose;
    }
    else
    {
        _world_pose.rotation = rotation;
        if( _parent )
            _pose = _world_pose / _parent->_world_pose;
        else
            _pose = _world_pose;
    }

    update_children();
}

void Transform::set_rotation(const Vector3f& rotation, TransformSpace space)
{
    set_rotation(from_euler_angles(rotation), space);
}

Vector3f Transform::get_scale(TransformSpace space) const
{
    if( TransformSpace::LOCAL == space )
        return _pose.scale;
    else
        return _world_pose.scale;
}

Vector3f Transform::get_position(TransformSpace space) const
{
    if( TransformSpace::LOCAL == space )
        return _pose.position;
    else
        return _world_pose.position;
}

Quaternion Transform::get_rotation(TransformSpace space) const
{
    if( TransformSpace::LOCAL == space )
        return _pose.rotation;
    else
        return _world_pose.rotation;
}

Vector3f Transform::transform_point(const Vector3f& point) const
{
    return ((point * _world_pose.rotation) * _world_pose.scale) + _world_pose.position;
}

Vector3f Transform::inverse_transform_point(const Vector3f& point) const
{
    return ((point - _world_pose.position) / _world_pose.scale) / _world_pose.rotation;
}

Vector3f Transform::transform_vector(const Vector3f& v) const
{
    return (v * _world_pose.rotation) * _world_pose.scale;
}

Vector3f Transform::inverse_transform_vector(const Vector3f& v) const
{
    return (v / _world_pose.scale) / _world_pose.rotation;
}

Vector3f Transform::transform_direction(const Vector3f& d) const
{
    return d * _world_pose.rotation;
}

Vector3f Transform::inverse_transform_direction(const Vector3f& d) const
{
    return d / _world_pose.rotation;
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
        transform._world_pose = _world_pose * transform._pose;
    else
        transform._pose = transform._world_pose / _world_pose;

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

Transform* Transform::get_root()
{
    if( get_parent() == nullptr )
        return nullptr;

    auto parent = get_parent();
    while( parent->get_parent() != nullptr )
        parent = parent->get_parent();
    return parent;
}

INLINE static Matrix4f calculate_model_matrix(const TransformPose& pose)
{
    auto matrix = (Matrix4f)to_rotation_matrix(pose.rotation);
    matrix *= (Matrix4f)math::scale(pose.scale);
    matrix *= translation(pose.position);
    return matrix;
}

Matrix4f Transform::get_model_matrix(TransformSpace space) const
{
    return calculate_model_matrix(TransformSpace::WORLD == space ? _world_pose : _pose);
}

INLINE static Matrix3f calculate_normal_matrix(const TransformPose& pose)
{
    return to_rotation_matrix(pose.rotation);
}

Matrix3f Transform::get_normal_matrix(TransformSpace space) const
{
    return calculate_normal_matrix(TransformSpace::WORLD == space ? _world_pose : _pose);
}

NS_LEMON_END
