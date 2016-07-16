// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/ui/element.hpp>
#include <scene/transform.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

void Widget::on_spawn(EntityManager& world, Entity object)
{
    ASSERT( world.has_component<Transform>(object),
        "Widget requires Transform component." );

    _transform = world.get_component<Transform>(object);
}

void Widget::on_dispose(EntityManager& world, Entity object)
{
    _transform = nullptr;
}

void Widget::set_visible(bool visible)
{
    _visible = visible;
}

bool Widget::is_visible(bool recursive) const
{
    if( !recursive )
        return _visible;

    for( auto& ancestor : _transform->get_ancestors() )
    {
        if( !ancestor.has_component<Widget>() )
            continue;

        auto ae = ancestor.get_component<Widget>();
        if( !ae->_visible )
            return false;
    }

    return true;
};

void Widget::set_anchor(const Vector2f& anchor)
{
    ENSURE( !isnan(anchor) && !isinf(anchor) );

    auto clamped = clamp(anchor, {0.f, 0.f}, {1.f, 1.f});
    auto position = (clamped - _anchor) * _custom_size + _transform->get_position();

    _anchor = clamped;
    _transform->set_position( position );
}

Vector2f Widget::get_anchor() const
{
    return _anchor;
}

void Widget::set_custom_size(const Vector2f& size)
{
    ENSURE( !isnan(size) && !isinf(size) );

    _custom_size = size;

    // update optional margin hints
    auto parent = _transform->get_parent();
    auto bounds = get_bounds();

    if( !parent || !parent->has_component<Widget>() )
    {
        _margin[0] = _margin[1] = _margin[2] = _margin[3] = math::nan;
    }
    else
    {
        auto widget = parent->get_component<Widget>();
        auto parent_bounds = widget->get_bounds();

        if( !std::isnan(_margin[0]) ) _margin[0] = bounds[0] - parent_bounds[0];
        if( !std::isnan(_margin[1]) ) _margin[1] = parent_bounds[2] - bounds[2];
        if( !std::isnan(_margin[2]) ) _margin[2] = parent_bounds[3] - bounds[3];
        if( !std::isnan(_margin[3]) ) _margin[3] = bounds[1] - parent_bounds[1];
    }
}

Vector2f Widget::get_custom_size() const
{
    return _custom_size;
}

Rect2f Widget::get_bounds() const
{
    auto position = _transform->get_position();
    return {
        position - _anchor*_custom_size,
        position + (Vector2f{1-_anchor[0], 1-_anchor[1]})*_custom_size };
}

void Widget::set_margin(WidgetEdge edge, float margin, bool is_percent)
{
    ASSERT( _transform->get_parent() && _transform->get_parent()->has_component<Widget>(),
        "it makes no sense to set widget's margin which has no parent." );

    auto size = _transform->get_parent()->get_component<Widget>()->get_custom_size();
    if( is_percent )
    {
        switch(edge)
        {
            case WidgetEdge::LEFT:
            case WidgetEdge::RIGHT: margin = size[0] * margin; break;
            case WidgetEdge::TOP:
            case WidgetEdge::BOTTOM:
            default: margin = size[1] * margin; break;
        }
    }

    _margin[static_cast<uint8_t>(edge)] = margin;
}

void Widget::on_resize(const Rect2f& bounds)
{
    if( !std::isnan(_margin[0]) && !std::isnan(_margin[1]) )
        _custom_size[0] = std::max(bounds.size()[0] - _margin[0] - _margin[1], 0.f);

    if( !std::isnan(_margin[2]) && !std::isnan(_margin[3]) )
        _custom_size[1] = std::max(bounds.size()[1] - _margin[2] - _margin[3], 0.f);

    auto position = _transform->get_position();
    if( !std::isnan(_margin[0]) )
        position[0] = (bounds[0] + _margin[0]) + _custom_size[0] * _anchor[0];
    else if( !std::isnan(_margin[1]) )
        position[0] = (bounds[2] - _margin[1]) - _custom_size[0] * (1-_anchor[0]);

    if( !std::isnan(_margin[2]) )
        position[1] = (bounds[3] - _margin[2]) - _custom_size[1] * (1-_anchor[1]);
    else if( !std::isnan(_margin[3]) )
        position[1] = (bounds[1] + _margin[3]) + _custom_size[1] * _anchor[1];
}

NS_FLOW2D_UI_END