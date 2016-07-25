// @date 2016/07/01
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/widget.hpp>
#include <scene/transform.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

void Widget::on_spawn(EntityManager& world, Entity object)
{
    ComponentWithEnvironment::on_spawn(world, object);
    ASSERT( world.has_component<Transform>(object),
        "Widget requires Transform component." );
}

bool Widget::is_inside(const Vector2f& position, TransformSpace space) const
{
    auto local = space == TransformSpace::WORLD ?
        get_component<Transform>()->inverse_transform_point(position) : position;
    return get_bounds_in(nullptr).is_inside(local);
}

void Widget::set_visible(bool visible)
{
    _visible = visible;
}

bool Widget::is_visible(bool recursive)
{
    if( !recursive )
        return _visible;

    for( auto& ancestor : get_component<Transform>()->get_ancestors() )
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

    auto transform = get_component<Transform>();
    auto clamped = clamp(anchor, {0.f, 0.f}, {1.f, 1.f});
    auto position = (clamped - _anchor) * _custom_size + transform->get_position();

    _anchor = clamped;
    transform->set_position( position );
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
    auto parent = get_component<Transform>()->get_parent();

    if( !parent || !parent->has_component<Widget>() )
    {
        _margin[0] = _margin[1] = _margin[2] = _margin[3] = math::nan<float>();
    }
    else
    {
        auto bounds = get_bounds_in(parent);
        auto parent_bounds = parent->get_component<Widget>()->get_bounds_in();

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

Rect2f Widget::get_bounds_in(Transform* target_transform) const
{
    auto transform = get_component<Transform>();
    if( target_transform == nullptr || target_transform == transform )
        return { -_anchor*_custom_size, _custom_size };

    // get position/scale in target-transform-space
    auto position = transform->get_position(TransformSpace::WORLD);
    position = target_transform->inverse_transform_point(position);

    auto scale = transform->get_scale(TransformSpace::WORLD);
    scale /= target_transform->get_scale(TransformSpace::WORLD);

    return { position - _anchor * _custom_size * scale, _custom_size * scale };
}

Rect2f Widget::get_bounds_in_world() const
{
    auto transform = get_component<Transform>();
    auto position = transform->get_position(TransformSpace::WORLD);
    auto scale = transform->get_scale(TransformSpace::WORLD);
    return { position - _anchor * _custom_size * scale, _custom_size * scale };
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

    auto transform = get_component<Transform>();
    auto position = transform->get_position();
    if( !std::isnan(_margin[0]) )
        position[0] = (bounds.lower<0>() + _margin[0]) + _custom_size[0] * _anchor[0];
    else if( !std::isnan(_margin[1]) )
        position[0] = (bounds.upper<0>() - _margin[1]) - _custom_size[0] * (1-_anchor[0]);

    if( !std::isnan(_margin[2]) )
        position[1] = (bounds.upper<1>() - _margin[2]) - _custom_size[1] * (1-_anchor[1]);
    else if( !std::isnan(_margin[3]) )
        position[1] = (bounds.lower<1>() + _margin[3]) + _custom_size[1] * _anchor[1];
    transform->set_position(position);
}

void View::on_spawn(EntityManager& world, Entity object)
{
    VTraitComponent::on_spawn(world, object);

    _world = &world;
    _object = object;

    ENSURE( has_component<View::Trait>() );
    ASSERT( has_component<Widget>(), "View requires Widget component." );
}

void View::on_dispose(EntityManager& world, Entity object)
{
    VTraitComponent::on_dispose(world, object);
    _world = nullptr;
    _object = Entity();
}

NS_FLOW2D_UI_END