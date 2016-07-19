// @date 2016/07/04
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/button.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

void Button::on_spawn(EntityManager& world, Entity object)
{
    VTraitComponent::on_spawn(world, object);

    if( !world.has_component<EventListenerGroup>(object) )
        world.add_component<EventListenerGroup>(object);

    auto group = world.get_component<EventListenerGroup>(object);
    group->subscribe<EvtMouseDown>(*this);
    group->subscribe<EvtMouseClick>(*this);
}

void Button::on_dispose(EntityManager& world, Entity object)
{
    VTraitComponent::on_dispose(world, object);

    if( !world.has_component<EventListenerGroup>(object) )
        world.add_component<EventListenerGroup>(object);

    auto group = world.get_component<EventListenerGroup>(object);
    group->unsubscribe<EvtMouseDown>(*this);
    group->unsubscribe<EvtMouseClick>(*this);
}

void Button::receive(Transform& transfrom, EvtMouseDown& evt)
{
    set_state(state::PRESSED);
}

void Button::receive(Transform& transfrom, EvtMouseClick& evt)
{
    set_state(state::NORMAL);
}

void Button::set_state(state s)
{
    _last_state = _current_state;
    _current_state = s;
    _fade_ticker = _fade_duration;
}

void Button::on_update(float dt)
{
    if( _fade_ticker > 0 )
    {
        _fade_ticker -= dt;
        _fade_ticker = std::max(_fade_ticker, 0.f);
    }
}

void Button::set_fade_duration(float duration)
{
    _fade_duration = std::max(duration, 0.01f);
}

void Button::on_draw(Canvas& canvas, const Rect2f& bounds)
{
    canvas.save();

    canvas.begin_path();
    canvas.move_to({bounds.lower<0>(), bounds.lower<1>()});
    canvas.line_to({bounds.upper<0>(), bounds.lower<1>()});
    canvas.line_to({bounds.upper<0>(), bounds.upper<1>()});
    canvas.line_to({bounds.lower<0>(), bounds.upper<1>()});
    canvas.close_path();

    auto c = _colors[static_cast<uint8_t>(_current_state)];
    auto l = _colors[static_cast<uint8_t>(_last_state)];
    auto f = _fade_ticker / _fade_duration;

    auto paint = CanvasPaint().as_linear_gradient(
        {bounds.lower<0>(), bounds.lower<1>()},
        {bounds.lower<0>(), bounds.upper<1>()},
        lerp(Color::GRAY*c, Color::GRAY*l, f),
        lerp(Color::WHITE*c, Color::WHITE*l, f));
    canvas.set_fill_paint(paint);
    canvas.fill();

    canvas.restore();
}

NS_FLOW2D_UI_END