// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

INLINE Transform::iterator& Transform::iterator::operator ++ ()
{
    if( _cusor == nullptr )
        return *this;

    if( _mode == iterator_mode::ANCESTORS )
    {
        _cusor = _cusor->_parent;
    }
    else if( _mode == iterator_mode::CHILDREN )
    {
        _cusor = _cusor->_next_sibling;
    }
    else if( _mode == iterator_mode::CHILDREN_RECURSIVE )
    {
        if( _cusor->_first_child )
            _cusor = _cusor->_first_child;
        else if( _cusor->_next_sibling )
            _cusor = _cusor->_next_sibling;
        else
        {
            bool found_available_node = false;
            while( _cusor->_parent != _start )
            {
                if( _cusor->_parent->_next_sibling )
                {
                    _cusor = _cusor->_parent->_next_sibling;
                    found_available_node = true;
                    break;
                }
                else
                    _cusor = _cusor->_parent;
            }

            if( !found_available_node )
                _cusor = nullptr;
        }
    }

    return *this;
}

INLINE bool Transform::iterator::operator == (const Transform::iterator& rh) const
{
    return _cusor == rh._cusor && _mode == rh._mode;
}

INLINE bool Transform::iterator::operator != (const Transform::iterator& rh) const
{
    return !(*this == rh);
}

INLINE Transform& Transform::iterator::operator * () const
{
    return *_cusor;
}

INLINE Transform::iterator Transform::view::begin() const
{
    return Transform::iterator(_start, _mode);
}

INLINE Transform::iterator Transform::view::end() const
{
    return Transform::iterator(nullptr, _mode);
}

INLINE void Transform::view::visit(const visitor& cb)
{
    for( auto iter = begin(); iter != end(); ++iter )
        cb(*iter);
}

INLINE size_t Transform::view::count() const
{
    size_t result = 0;
    for( auto iter = begin(); iter != end(); ++iter ) result++;
    return result;
}

template<typename ... T>
INLINE void Transform::view_trait<T...>::visit(const visitor& cb)
{
    for( auto iter = begin(); iter != end(); ++ iter )
    {
        auto mask = _start->_world->get_components_mask(_start->_object);
        if( (mask & _mask) == _mask )
            cb(*iter, *((*iter).template get_component<T>()) ...);
    }
}

template<typename ... T>
INLINE size_t Transform::view_trait<T...>::count() const
{
    size_t result = 0;
    for( auto iter = begin(); iter != end(); ++ iter )
    {
        auto mask = _start->_world->get_components_mask(_start->_object);
        if( (mask & _mask) == _mask ) result++;
    }
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

INLINE void Transform::scale(const Vector2f& scaler)
{
    set_scale(get_scale() * scaler);
}

INLINE void Transform::rotate(float rotation)
{
    set_rotation(get_rotation() + rotation);
}

INLINE void Transform::translate(const Vector2f& translation)
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

template<typename ... T>
INLINE Transform::view_trait<T...> Transform::get_children_with(bool recursive)
{
    return view_trait<T...>(this,
        recursive ? iterator_mode::CHILDREN_RECURSIVE : iterator_mode::CHILDREN );
}

INLINE Matrix3f Transform::to_matrix(TransformSpace space) const
{
    if( TransformSpace::WORLD == space )
    {
        auto matrix = make_translation(_worldspace.position);
        matrix *= hlift(make_rotation(_worldspace.rotation));
        matrix *= hlift(make_scale(_worldspace.scale));
        return matrix;
    }
    else
    {
        auto matrix = make_translation(_localspace.position);
        matrix *= hlift(make_rotation(_localspace.rotation));
        matrix *= hlift(make_scale(_localspace.scale));
        return matrix;
    }
}
