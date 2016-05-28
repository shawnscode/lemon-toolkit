// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include "transform.hpp"

NS_FLOW2D_BEGIN

Transform::Transform()
{
    reset();
}

Transform::Transform(Transform* parent)
{
    reset();
    set_parent(parent);
}

Transform::Transform(Transform* parent, const glm::vec2& position, const glm::vec2& scale, float rotation)
{
    reset();
    m_position = position;
    m_scale = scale;
    m_rotation = rotation;
    m_transform = glm::scale(glm::mat3(1.0f), m_scale);
    m_transform = glm::rotate(m_transform, m_rotation);
    m_transform = glm::translate(m_transform, m_position);
    set_parent(parent);
}

Transform* Transform::set_parent(Transform* parent)
{
    if( m_parent != nullptr )
    {
        if( m_parent->m_first_child == this )
        {
            assert( m_prev_sibling == nullptr );
            if( m_next_sibling )
                m_next_sibling->m_prev_sibling = nullptr;
            m_parent->m_first_child = m_next_sibling;
        }
    }

    m_parent = parent;
    m_next_sibling = m_parent->m_first_child;
    if( m_next_sibling ) m_next_sibling->m_prev_sibling = this;
    m_parent->m_first_child = this;

    return this;
}

Transform* Transform::find_with_name(const char* name)
{
    auto size = strlen(name);
    auto p = strchr(name, '.');
    if( p != nullptr )
    {
        *p = '\0';
        for( auto cursor = m_first_child; cursor != nullptr; cursor = cursor->m_next_sibling )
            if( cursor->m_name == name )
                return cursor->find_with_name(p+1);
    }

    for( auto cursor = m_first_child; cursor != nullptr; cursor = cursor->m_next_sibling )
        if( cursor->m_name == name )
            return cursor;

    return nullptr;
}

NS_FLOW2D_END