// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/widget.hpp>
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

bool Widget::is_inside(const Vector2f& position, TransformSpace space) const
{
    auto local = space == TransformSpace::WORLD ? _transform->inverse_transform_point(position) : position;
    return Rect2f( { -_anchor*_custom_size, _custom_size } ).is_inside(local);
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
        _margin[0] = _margin[1] = _margin[2] = _margin[3] = math::nan<float>();
    }
    else
    {
        auto widget = parent->get_component<Widget>();
        auto parent_bounds = widget->get_bounds();

        if( !std::isnan(_margin[0]) )
            _margin[0] = bounds.lower<0>() - parent_bounds.lower<0>();

        if( !std::isnan(_margin[1]) )
            _margin[1] = parent_bounds.upper<0>() - bounds.upper<0>();

        if( !std::isnan(_margin[2]) )
            _margin[2] = parent_bounds.upper<1>() - bounds.upper<1>();

        if( !std::isnan(_margin[3]) )
            _margin[3] = bounds.lower<1>() - parent_bounds.lower<1>();
    }
}

Vector2f Widget::get_custom_size() const
{
    return _custom_size;
}

Rect2f Widget::get_bounds() const
{
    auto position = _transform->get_position();
    return { position - _anchor*_custom_size, _custom_size };
}

void Widget::set_margin(WidgetEdge edge, float margin)
{
    _margin[static_cast<uint8_t>(edge)] = margin;
}

void Widget::perform_resize(const Rect2f& bounds)
{
    if( !std::isnan(_margin[0]) && !std::isnan(_margin[1]) )
        _custom_size[0] = std::max(bounds.length<0>() - _margin[0] - _margin[1], 0.f);

    if( !std::isnan(_margin[2]) && !std::isnan(_margin[3]) )
        _custom_size[1] = std::max(bounds.length<1>() - _margin[2] - _margin[3], 0.f);

    auto position = _transform->get_position();
    if( !std::isnan(_margin[0]) )
        position[0] = (bounds.lower<0>() + _margin[0]) + _custom_size[0] * _anchor[0];
    else if( !std::isnan(_margin[1]) )
        position[0] = (bounds.upper<0>() - _margin[1]) - _custom_size[0] * (1-_anchor[0]);

    if( !std::isnan(_margin[2]) )
        position[1] = (bounds.upper<1>() - _margin[2]) - _custom_size[1] * (1-_anchor[1]);
    else if( !std::isnan(_margin[3]) )
        position[1] = (bounds.lower<1>() + _margin[3]) + _custom_size[1] * _anchor[1];
    _transform->set_position(position);
}

NS_FLOW2D_UI_END