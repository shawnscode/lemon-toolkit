// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/ui/element.hpp>
#include <scene/transform.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_BEGIN

void IViewController::on_spawn(EntityManager& world, Entity object)
{
    ASSERT( world.has_component<UIElement>(object),
        "interaction component of ui requires UIElement." );

    auto element = world.get_component<UIElement>(object);
    ASSERT( element->_view == nullptr,
        "duplicated interaction component of ui in same Entity." );

    element->_view = this;
}

void IViewController::on_dispose(EntityManager& world, Entity object)
{
    ENSURE( world.has_component<UIElement>(object) );

    auto element = world.get_component<UIElement>(object);
    ENSURE( element->_view == this );

    element->_view = nullptr;
}

void ILayout::on_spawn(EntityManager& world, Entity object)
{
    ASSERT( world.has_component<UIElement>(object),
        "layout component of ui requires UIElement." );

    auto element = world.get_component<UIElement>(object);
    ASSERT( element->_layout == nullptr,
        "duplicated layout component of ui in same Entity." );

    element->_layout = this;
}

void ILayout::on_dispose(EntityManager& world, Entity object)
{
    ENSURE( world.has_component<UIElement>(object) );

    auto element = world.get_component<UIElement>(object);
    ENSURE( element->_layout == this );

    element->_layout = nullptr;
}

void UIElement::on_spawn(EntityManager& world, Entity object)
{
    ASSERT( world.has_component<Transform>(object),
        "UIElement requires Transform component." );

    _transform = world.get_component<Transform>(object);
}

void UIElement::on_dispose(EntityManager& world, Entity object)
{
    _transform = nullptr;
}

bool UIElement::is_visible(bool recursive) const
{
    if( !recursive )
        return _visible;

    for( auto& ancestor : _transform->get_ancestors() )
    {
        if( !ancestor.has_component<UIElement>() )
            continue;

        auto ae = ancestor.get_component<UIElement>();
        if( !ae->_visible )
            return false;
    }

    return true;
};

void UIElement::set_visible(bool visible)
{
    _visible = visible;
}

bool UIElement::has_fixed_size() const
{
    return !isnan(_fixed_size);
}

void UIElement::set_fixed_size(const Vector2f& size)
{
    _fixed_size = size;
}

Vector2f UIElement::get_fixed_size() const
{
    return _fixed_size;
}

Vector2f UIElement::get_prefered_size() const
{
    if( _layout )
        return _layout->get_prefered_size();

    return _size;
}

Vector2f UIElement::get_size() const
{
    return _size;
}

Rect2f UIElement::get_bounds() const
{
    auto position = _transform->get_position(TransformSpace::WORLD);
    auto ll = position - Vector2f { _size[0] * _pivot[0], _size[1] * _pivot[1] };
    auto ru = ll + _size;
    return Rect2f { ll[0], ll[1], ru[0], ru[1] };
}

void UIElement::rearrange()
{
    if( _layout )
    {
        _layout->perform(*_transform);
        return;
    }

    _transform->get_children_with<UIElement>().visit([&](Transform& ct, UIElement& ce)
    {
        if( ce.has_fixed_size() )
            ce._size = ce.get_fixed_size();
        else
            ce._size = ce.get_prefered_size();
        ce.rearrange();
    });
}

void UIElement::draw(Canvas& canvas)
{
    if( _view )
    {
        _view->draw(*this, canvas);
        return;
    }

    if( _transform->is_leaf() )
        return;

    _transform->get_children_with<UIElement>().visit([&](Transform& ct, UIElement& ce)
    {
        if( ce.is_visible() )
            ce.draw(canvas);
    });
}

NS_FLOW2D_END