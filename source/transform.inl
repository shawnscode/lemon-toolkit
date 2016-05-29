// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF TRANSFORM ITERATOR
INLINE TransformIterator& TransformIterator::operator ++ ()
{
    if( m_cursor )
        m_cursor = m_cursor->m_next_sibling;
    return *this;
}

INLINE bool TransformIterator::operator == (const TransformIterator& rh) const
{
    return m_cursor == rh.m_cursor;
}

INLINE bool TransformIterator::operator != (const TransformIterator& rh) const
{
    return m_cursor != rh.m_cursor;
}

Transform* TransformIterator::operator * ()
{
    return m_cursor;
}

const Transform* TransformIterator::operator * () const
{
    return m_cursor;
}

// INCLUDED METHODS OF TRANSFORM
INLINE void Transform::reset()
{
    m_position.x = m_position.y = 0.f;
    m_scale.x = m_scale.y = 1.0f;
    m_rotation = 0.f;
    m_transform = glm::mat3(1.f);
    m_update_transform = false;
}

INLINE void Transform::set_scale(const glm::vec2& scale)
{
    m_scale = scale;
    m_update_transform = true;
}

INLINE void Transform::set_position(const glm::vec2& position)
{
    m_position = position;
    m_update_transform = true;
}

INLINE void Transform::set_rotation(float degree)
{
    m_rotation = degree;
    m_update_transform = true;
}

INLINE glm::vec2 Transform::get_scale() const
{
    return m_scale;
}

INLINE glm::vec2 Transform::get_position() const
{
    return m_position;
}

INLINE float Transform::get_rotation() const
{
    return m_rotation;
}

INLINE void Transform::translate(const glm::vec2& offset)
{
    set_position(m_position + offset);
}

INLINE void Transform::rotate(float degree)
{
    set_rotation(m_rotation + degree);
}

INLINE const glm::mat3& Transform::get_transform() const
{
    if( m_update_transform )
    {
        m_transform = glm::scale(glm::mat3(1.0f), m_scale);
        m_transform = glm::rotate(m_transform, m_rotation);
        m_transform = glm::translate(m_transform, m_position);
        m_update_transform = false;
    }
    return m_transform;
}

INLINE Transform* Transform::get_parent()
{
    return m_parent;
}

INLINE size_t Transform::get_child_count() const
{
    size_t size = 0;
    auto cursor = m_first_child;
    while(cursor != nullptr)
    {
        size ++;
        cursor = cursor->m_next_sibling;
    }
    return size;
}

INLINE TransformIterator Transform::begin()
{
    return TransformIterator(m_first_child);
}

INLINE TransformIterator Transform::end()
{
    return TransformIterator(nullptr);
}

INLINE const TransformIterator Transform::begin() const
{
    return TransformIterator(m_first_child);
}

INLINE const TransformIterator Transform::end() const
{
    return TransformIterator(nullptr);
}

INLINE void Transform::set_name(const char* src)
{
    m_name = src;
}

INLINE const char* Transform::get_name() const
{
    return m_name.c_str();
}

INLINE void Transform::set_scene(SceneSystem* scene)
{
    assert( m_scene == nullptr && "[TRANSFORM] transform has already been attached to scene.");
    m_scene = scene;
}