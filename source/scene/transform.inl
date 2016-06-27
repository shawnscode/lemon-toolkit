// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF TRANSFORM ITERATOR
INLINE TransformIterator& TransformIterator::operator ++ ()
{
    if( cursor )
        cursor = cursor->next_sibling;
    return *this;
}

INLINE bool TransformIterator::operator == (const TransformIterator& rh) const
{
    return cursor == rh.cursor;
}

INLINE bool TransformIterator::operator != (const TransformIterator& rh) const
{
    return cursor != rh.cursor;
}

INLINE TransformComponent* TransformIterator::operator * ()
{
    return cursor;
}

INLINE const TransformComponent* TransformIterator::operator * () const
{
    return cursor;
}

INLINE Transform Transform::operator / (const Transform& rh) const
{
    return Transform(
        position - rh.position,
        { scale[0] / rh.scale[0], scale[1] / rh.scale[1] },
        rotation - rh.rotation);
}

INLINE Transform Transform::operator * (const Transform& rh) const
{
    return Transform(
        position + rh.position,
        { scale[0] * rh.scale[0], scale[1] * rh.scale[1] },
        rotation + rh.rotation);
}

// INCLUDED METHODS OF TRANSFORM
INLINE void TransformComponent::identity()
{
    transform = Transform::IDENTITY;
    update_children();
}

INLINE Transform TransformComponent::get_transform(TransformSpace space) const
{
    return TransformSpace::SELF == space ? transform : world_transform;
}

INLINE void TransformComponent::set_transform(const Transform& pose, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        transform = pose;
        if( parent ) world_transform = parent->world_transform * transform;
    }
    else
    {
        world_transform = pose;
        if( parent ) transform = world_transform / parent->world_transform;
    }

    update_children();
}

INLINE void TransformComponent::set_scale(const Vector2f& scale, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        transform.scale = scale;
        if( parent ) world_transform = parent->world_transform * transform;
    }
    else
    {
        world_transform.scale = scale;
        if( parent ) transform = world_transform / parent->world_transform;
    }

    update_children();
}

INLINE void TransformComponent::set_position(const Vector2f& position, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        transform.position = position;
        if( parent ) world_transform = parent->world_transform * transform;
    }
    else
    {
        world_transform.position = position;
        if( parent ) transform = world_transform / parent->world_transform;
    }

    update_children();
}

INLINE void TransformComponent::set_rotation(float rotation, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        transform.rotation = rotation;
        if( parent ) world_transform = parent->world_transform * transform;
    }
    else
    {
        world_transform.rotation = rotation;
        if( parent ) transform = world_transform / parent->world_transform;
    }

    update_children();
}

INLINE Vector2f TransformComponent::get_scale(TransformSpace space) const
{
    return space == TransformSpace::SELF ? transform.scale : world_transform.scale;
}

INLINE Vector2f TransformComponent::get_position(TransformSpace space) const
{
    return space == TransformSpace::SELF ? transform.position : world_transform.position;
}

INLINE float TransformComponent::get_rotation(TransformSpace space) const
{
    return space == TransformSpace::SELF ? transform.rotation : world_transform.rotation;
}

INLINE TransformComponent* TransformComponent::get_parent()
{
    return parent;
}

INLINE size_t TransformComponent::get_child_count() const
{
    size_t size = 0;
    auto cursor = first_child;
    while(cursor != nullptr)
    {
        size ++;
        cursor = cursor->next_sibling;
    }
    return size;
}

INLINE TransformIterator TransformComponent::begin()
{
    return TransformIterator(first_child);
}

INLINE TransformIterator TransformComponent::end()
{
    return TransformIterator(nullptr);
}

INLINE const TransformIterator TransformComponent::begin() const
{
    return TransformIterator(first_child);
}

INLINE const TransformIterator TransformComponent::end() const
{
    return TransformIterator(nullptr);
}