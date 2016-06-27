// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/transform.hpp>

NS_FLOW2D_BEGIN

Transform::Transform() {}

Transform::Transform(const Vector2f& position, const Vector2f& scale, float rotation)
{
    local.position = position;
    local.scale = scale;
    local.rotation = rotation;
}

Transform* Transform::set_parent(Transform* parent)
{
    ASSERT( parent, "[Transform] parent = nullptr." );
    remove_from_parent();

    if( parent->first_child != nullptr )
    {
        ENSURE( parent->first_child->prev_sibling == nullptr );
        parent->first_child->prev_sibling = this;
        this->next_sibling = parent->first_child;
    }

    this->parent = parent;
    parent->first_child = this;
    this->world = parent->world * this->local;

    update_children();
    return this;
}

void Transform::remove_from_parent()
{
    if( parent )
    {
        if( parent->first_child == this )
        {
            ENSURE( prev_sibling == nullptr );
            if( next_sibling )
            {
                ENSURE( next_sibling->prev_sibling == this );
                next_sibling->prev_sibling = nullptr;
            }
            parent->first_child = next_sibling;
        }
    }

    local = world;
}

void Transform::update_children()
{
    auto cursor = first_child;
    while( cursor )
    {
        cursor->set_transform(world * cursor->local, TransformSpace::WORLD);
        cursor = cursor->next_sibling;
    }
}

NS_FLOW2D_END