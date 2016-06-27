// @date 2016/05/26
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/transform.hpp>

NS_FLOW2D_BEGIN

TransformComponent::TransformComponent() {}

TransformComponent::TransformComponent(const Vector2f& position, const Vector2f& scale, float rotation)
{
    transform.position = position;
    transform.scale = scale;
    transform.rotation = rotation;
}

TransformComponent* TransformComponent::set_parent(TransformComponent* parent)
{
    ASSERT( parent, "[TransformComponent] parent = nullptr." );
    remove_from_parent();

    if( parent->first_child != nullptr )
    {
        ENSURE( parent->first_child->prev_sibling == nullptr );
        parent->first_child->prev_sibling = this;
        this->next_sibling = parent->first_child;
    }

    this->parent = parent;
    parent->first_child = this;
    this->world_transform = parent->world_transform * this->transform;

    update_children();
    return this;
}

void TransformComponent::remove_from_parent()
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

    transform = world_transform;
}

void TransformComponent::update_children()
{
    auto cursor = first_child;
    while( cursor )
    {
        cursor->set_transform(world_transform * cursor->transform, TransformSpace::WORLD);
        cursor = cursor->next_sibling;
    }
}

NS_FLOW2D_END