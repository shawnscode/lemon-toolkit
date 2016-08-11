// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

/// IMPLEMNTATIONS OF TRANSFORM POSE
INLINE TransformPose operator * (const TransformPose& lhs, const TransformPose& rhs)
{
    auto result = lhs;
    return result *= rhs;
}

INLINE TransformPose operator / (const TransformPose& lhs, const TransformPose& rhs)
{
    auto result = lhs;
    return result /= rhs;
}

INLINE TransformPose& operator *= (TransformPose& lhs, const TransformPose& rhs)
{
    lhs.position += rhs.position;
    lhs.scale    *= rhs.scale;
    lhs.rotation += rhs.rotation;
    return lhs;
}

INLINE TransformPose& operator /= (TransformPose& lhs, const TransformPose& rhs)
{
    lhs.position -= rhs.position;
    lhs.scale    /= rhs.scale;
    lhs.rotation -= rhs.rotation;
    return lhs;
}

/// IMPLEMENTATONS OF TRANSFORM ITERATOR
template<typename T>
Transform::iterator_t<T>::iterator_t(T* transform, iterate_mode mode, core::ComponentMask mask)
:_start(transform), _mode(mode), _mask(mask)
{
    if( transform != nullptr )
    {
        _start = transform;
        if( mode == iterate_mode::ANCESTORS )
            _cursor = transform->_parent;
        else
            _cursor = transform->_first_child;
        
        if( _cursor != nullptr && (_cursor->get_components_mask() & _mask) != _mask )
            find_next_available();
    }
    else
        _start = _cursor = nullptr;
}

template<typename T>
void Transform::iterator_t<T>::find_next_available()
{
    if( _cursor == nullptr )
        return;

    if( _mode == iterate_mode::ANCESTORS )
    {
        _cursor = _cursor->_parent;
        return;
    }

    if( _mode == iterate_mode::CHILDREN )
    {
        _cursor = _cursor->_next_sibling;
        return;
    }

    // deep first search when iterating children recursively
    if( _cursor->_first_child )
    {
        _cursor = _cursor->_first_child;
        return;
    }

    if( _cursor->_next_sibling )
    {
        _cursor = _cursor->_next_sibling;
        return;
    }

    // travel back when we reach leaf-node
    bool found_available_node = false;
    while( _cursor->_parent != _start )
    {
        if( _cursor->_parent->_next_sibling )
        {
            _cursor = _cursor->_parent->_next_sibling;
            found_available_node = true;
            break;
        }
        else
            _cursor = _cursor->_parent;
    }

    if( !found_available_node )
        _cursor = nullptr;
    return;
}

template<typename T>
INLINE Transform::iterator_t<T>& Transform::iterator_t<T>::operator ++ ()
{
    while( _cursor != nullptr )
    {
        find_next_available();
        if( (_cursor == nullptr) || (_cursor->get_components_mask() & _mask) == _mask )
            break;
    }
    return *this;
}

template<typename T>
INLINE Transform::iterator_t<T> Transform::iterator_t<T>::operator ++ (int dummy)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T>
INLINE bool Transform::iterator_t<T>::operator == (const Transform::iterator_t<T>& rh) const
{
    return _cursor == rh._cursor && _mode == rh._mode;
}

template<typename T>
INLINE bool Transform::iterator_t<T>::operator != (const Transform::iterator_t<T>& rh) const
{
    return !(*this == rh);
}

template<typename T>
INLINE T& Transform::iterator_t<T>::operator * ()
{
    return *_cursor;
}

/// IMPLEMENTATIONS OF TRANSFORM VIEW
template<typename T, typename ... Args>
Transform::view_t<T, Args...>::view_t(T* transform, iterate_mode mode)
: _transform(transform), _mode(mode)
{
    //_mask = transform->get_world()->template get_components_mask<Args...>();
}

template<typename T, typename ... Args>
INLINE Transform::iterator_t<T> Transform::view_t<T, Args...>::begin() const
{
    return iterator_t<T>(_transform, _mode, _mask);
}

template<typename T, typename ... Args>
INLINE Transform::iterator_t<T> Transform::view_t<T, Args...>::end() const
{
    return iterator_t<T>(nullptr, _mode, _mask);
}

template<typename T, typename ... Args>
INLINE void Transform::view_t<T, Args...>::visit(const visitor& cb)
{
    for( auto& iterator : *this ) cb(iterator);
}

template<typename T, typename ... Args>
INLINE unsigned Transform::view_t<T, Args...>::count() const
{
    unsigned result = 0;
    for( auto& _iterator : *this ) result++;
    return result;
}

INLINE bool Transform::operator == (const Transform& rh) const
{
    return get_object() == rh.get_object();
}

INLINE bool Transform::operator != (const Transform& rh) const
{
    return get_object() != rh.get_object();
}

INLINE void Transform::scale(const Vector3f& scaler)
{
    set_scale(get_scale() * scaler);
}

INLINE void Transform::rotate(const Vector3f& rotation)
{
    set_rotation(get_rotation() + rotation);
}

INLINE void Transform::translate(const Vector3f& translation)
{
    set_position(get_position() + translation);
}

INLINE bool Transform::is_root() const
{
    return _parent == nullptr;
}

INLINE bool Transform::is_leaf() const
{
    return _first_child == nullptr;
}

INLINE Transform* Transform::get_parent()
{
    return _parent;
}

INLINE const Transform* Transform::get_parent() const
{
    return _parent;
}

template<typename ... T>
INLINE Transform::view<T...> Transform::get_children_with(bool recursive)
{
    return view<T...>(this,
        recursive ? iterate_mode::CHILDREN_RECURSIVE : iterate_mode::CHILDREN );
}

INLINE Matrix4f Transform::to_matrix(TransformSpace space) const
{
    if( TransformSpace::WORLD == space )
    {
        auto matrix = translation(_world_pose.position);
        // matrix *= hlift(rotation(_world_pose.rotation));
        // matrix *= hlift(scale(_world_pose.scale));
        return matrix;
    }
    else
    {
        auto matrix = translation(_pose.position);
        // matrix *= hlift(rotation(_pose.rotation));
        // matrix *= hlift(scale(_pose.scale));
        return matrix;
    }
}
