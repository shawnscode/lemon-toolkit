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
    lhs.rotation *= rhs.rotation;
    return lhs;
}

INLINE TransformPose& operator /= (TransformPose& lhs, const TransformPose& rhs)
{
    lhs.position -= rhs.position;
    lhs.scale    /= rhs.scale;
    lhs.rotation /= rhs.rotation;
    return lhs;
}

template<typename T> T* Transform::find_parent(T* current)
{
    return current == nullptr ? nullptr : current->_parent;
}

template<typename T> T* Transform::find_next_children(T* root, T* current)
{
    if( root == nullptr || current == nullptr )
        return nullptr;

    return current == root ? current->_first_child : current->_next_sibling;
}

template<typename T> T* Transform::find_next_children_recursive(T* root, T* current)
{
    if( root == nullptr || current == nullptr )
        return nullptr;

    // deep first search when iterating children recursively
    if( current->_first_child )
        return current->_first_child;

    if( current->_next_sibling )
        return current->_next_sibling;

    // travel back when we reach leaf-node
    while( current->_parent != nullptr && current->_parent != root )
    {
        if( current->_parent->_next_sibling )
            return current->_parent->_next_sibling;
        else
            current = current->_parent;
    }

    return nullptr;
}

template<typename T> T* Transform::find_with_mask(const it_function<T>& cb, T* current, Component::Mask mask)
{
    while( current != nullptr )
    {
        current = cb(current);
        if( current == nullptr || (current->entity.get_components_mask() & mask) == mask )
            return current;
    }
    return nullptr;
}

/// IMPLEMENTATONS OF TRANSFORM ITERATOR
template<typename T>
Transform::iterator_traits<T>::iterator_traits(T* transform, const it_function<T>& func, core::Component::Mask mask)
: _current(transform), _iterator(func), _mask(mask)
{}

template<typename T>
INLINE Transform::iterator_traits<T>& Transform::iterator_traits<T>::operator ++ ()
{
    _current = Transform::find_with_mask(_iterator, _current, _mask);
    return *this;
}

template<typename T>
INLINE Transform::iterator_traits<T> Transform::iterator_traits<T>::operator ++ (int dummy)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T>
INLINE bool Transform::iterator_traits<T>::operator == (const Transform::iterator_traits<T>& rhs) const
{
    return _current == rhs._current;
}

template<typename T>
INLINE bool Transform::iterator_traits<T>::operator != (const Transform::iterator_traits<T>& rhs) const
{
    return !(*this == rhs);
}

template<typename T>
INLINE T* Transform::iterator_traits<T>::operator * ()
{
    return _current;
}

template<typename T, typename ... Args>
Transform::view_traits<T, Args...>::view_traits(T* transform, const it_function<T>& cb)
: _transform(transform), _iterator(cb)
{
    _mask = core::Component::calculate<Args...>();
}

template<typename T, typename ... Args>
INLINE Transform::iterator_traits<T> Transform::view_traits<T, Args...>::begin() const
{
    return iterator_traits<T>(Transform::find_with_mask(_iterator, _transform, _mask), _iterator, _mask);
}

template<typename T, typename ... Args>
INLINE Transform::iterator_traits<T> Transform::view_traits<T, Args...>::end() const
{
    return iterator_traits<T>(nullptr, _iterator, _mask);
}

template<typename T, typename ... Args>
INLINE void Transform::view_traits<T, Args...>::visit(const std::function<void(Transform&, Args&...)>& cb)
{
    for( auto iterator : *this )
        cb(*iterator, *((*iterator)->entity.template get_component<Args>())...);
}

template<typename T, typename ... Args>
INLINE unsigned Transform::view_traits<T, Args...>::count() const
{
    unsigned result = 0;
    for( auto _iterator : *this )
        result++;
    return result;
}

INLINE bool Transform::operator == (const Transform& rhs) const
{
    return &entity == &rhs.entity;
}

INLINE bool Transform::operator != (const Transform& rhs) const
{
    return &entity != &rhs.entity;
}

INLINE void Transform::scale(const Vector3f& scaler)
{
    set_scale(get_scale() * scaler);
}

INLINE void Transform::rotate(const Quaternion& rotation)
{
    set_rotation(get_rotation() * rotation);
}

INLINE void Transform::translate(const Vector3f& translation)
{
    set_position(get_position() + translation);
}

INLINE Vector3f Transform::get_forward(TransformSpace space) const
{
    return Vector3f {0.f, 0.f, 1.f} * get_rotation(space);
}

INLINE Vector3f Transform::get_back(TransformSpace space) const
{
    return Vector3f {0.f, 0.f, -1.f} * get_rotation(space);
}

INLINE Vector3f Transform::get_up(TransformSpace space) const
{
    return Vector3f {0.f, 1.f, 0.f} * get_rotation(space);
}

INLINE Vector3f Transform::get_down(TransformSpace space) const
{
    return Vector3f {0.f, -1.f, 0.f} * get_rotation(space);
}

INLINE Vector3f Transform::get_right(TransformSpace space) const
{
    return Vector3f {1.f, 0.f, 0.f} * get_rotation(space);
}

INLINE Vector3f Transform::get_left(TransformSpace space) const
{
    return Vector3f {-1.f, 0.f, 0.f} * get_rotation(space);
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
INLINE Transform::view<T...> Transform::find_children_with(bool recursive)
{
   return view<T...>(this, std::bind(recursive ?
        &Transform::find_next_children_recursive<Transform> : &Transform::find_next_children<Transform>,
        this,
        std::placeholders::_1));
}

template<typename ... T>
INLINE Transform::const_view<T...> Transform::find_children_with(bool recursive) const
{
   return const_view<T...>(this, std::bind( recursive ?
        &Transform::find_next_children_recursive<const Transform> : &Transform::find_next_children<const Transform>,
        this,
        std::placeholders::_1));
}

INLINE Matrix4f Transform::to_matrix(TransformSpace space) const
{
    if( TransformSpace::WORLD == space )
    {
        auto matrix = (Matrix4f)to_rotation_matrix(_world_pose.rotation);
        matrix *= (Matrix4f)math::scale(_world_pose.scale);
        matrix *= translation(_world_pose.position);
        return matrix;
    }
    else
    {
        auto matrix = (Matrix4f)to_rotation_matrix(_pose.rotation);
        matrix *= (Matrix4f)math::scale(_pose.scale);
        matrix *= translation(_pose.position);
        return matrix;
    }
}
