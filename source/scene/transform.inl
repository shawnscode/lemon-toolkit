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

INLINE Transform* TransformIterator::operator * ()
{
    return cursor;
}

INLINE const Transform* TransformIterator::operator * () const
{
    return cursor;
}

INLINE TransformMatrix TransformMatrix::operator / (const TransformMatrix& rh) const
{
    return TransformMatrix(
        position - rh.position,
        { scale[0] / rh.scale[0], scale[1] / rh.scale[1] },
        rotation - rh.rotation);
}

INLINE TransformMatrix TransformMatrix::operator * (const TransformMatrix& rh) const
{
    return TransformMatrix(
        position + rh.position,
        { scale[0] * rh.scale[0], scale[1] * rh.scale[1] },
        rotation + rh.rotation);
}

// INCLUDED METHODS OF TRANSFORM
INLINE void Transform::identity()
{
    local = TransformMatrix::IDENTITY;
    if( parent ) world = parent->world;
    update_children();
}

INLINE TransformMatrix Transform::get_transform(TransformSpace space) const
{
    return TransformSpace::SELF == space ? local : world;
}

INLINE void Transform::set_transform(const TransformMatrix& pose, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        local = pose;
        if( parent ) world = parent->world * local;
    }
    else
    {
        world = pose;
        if( parent ) local = world / parent->world;
    }

    update_children();
}

INLINE void Transform::set_scale(const Vector2f& scale, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        local.scale = scale;
        if( parent ) world = parent->world * local;
    }
    else
    {
        world.scale = scale;
        if( parent ) local = world / parent->world;
    }

    update_children();
}

INLINE void Transform::set_position(const Vector2f& position, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        local.position = position;
        if( parent ) world = parent->world * local;
    }
    else
    {
        world.position = position;
        if( parent ) local = world / parent->world;
    }

    update_children();
}

INLINE void Transform::set_rotation(float rotation, TransformSpace space)
{
    if( TransformSpace::SELF == space )
    {
        local.rotation = rotation;
        if( parent ) world = parent->world * local;
    }
    else
    {
        world.rotation = rotation;
        if( parent ) local = world / parent->world;
    }

    update_children();
}

INLINE Vector2f Transform::get_scale(TransformSpace space) const
{
    return space == TransformSpace::SELF ? local.scale : world.scale;
}

INLINE Vector2f Transform::get_position(TransformSpace space) const
{
    return space == TransformSpace::SELF ? local.position : world.position;
}

INLINE float Transform::get_rotation(TransformSpace space) const
{
    return space == TransformSpace::SELF ? local.rotation : world.rotation;
}

INLINE Transform* Transform::get_parent()
{
    return parent;
}

INLINE size_t Transform::get_child_count() const
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

INLINE TransformIterator Transform::begin()
{
    return TransformIterator(first_child);
}

INLINE TransformIterator Transform::end()
{
    return TransformIterator(nullptr);
}