// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/director.hpp>
#include <ui/listener.hpp>
#include <scene/transform.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

const static float kMoveThreshold = 0.1f;

CanvasDirector::CanvasDirector()
{
    resolve();
}

void CanvasDirector::set_resolve_mode(ResolutionResolveMode m)
{
    _resolve_mode = m;
    resolve();
}

void CanvasDirector::set_screen_size(const Vector2f& size)
{
    _screen_size = max(size, {1.f, 1.f});
    resolve();
}

void CanvasDirector::set_design_size(const Vector2f& size)
{
    _design_size = max(size, {1.f, 1.f});
    resolve();
}

Matrix3f CanvasDirector::get_ortho() const
{
    return _ortho;
}

Vector2f CanvasDirector::get_resolved_size() const
{
    return _resolved_size;
}

void CanvasDirector::resolve()
{
    switch( _resolve_mode )
    {
        case ResolutionResolveMode::SCALE_NO_BORDER:
        {
            _resolved_size = _design_size;
            if( _screen_size[0]/_screen_size[1] < _design_size[0]/_design_size[1] )
            {
                float width = _design_size[1] * _screen_size[0] / _screen_size[1];
                float offset = (_design_size[0] - width) * 0.5f;
                _ortho = make_ortho(offset, width+offset, 0.f, _design_size[1]);
            }
            else
            {
                float height = _design_size[0] * _screen_size[1] / _screen_size[0];
                float offset = (_design_size[1] - height) * 0.5f;
                _ortho = make_ortho(0.f, _design_size[0], offset, height+offset);
            }
            break;
        }

        case ResolutionResolveMode::SCALE_SHOW_ALL:
        {
            _resolved_size = _design_size;
            if( _screen_size[0]/_screen_size[1] < _design_size[0]/_design_size[1] )
            {
                float height = _design_size[0] * _screen_size[1]/_screen_size[0];
                float offset = (height - _design_size[1])*0.5f;
                _ortho = make_ortho(0.f, _design_size[0], -offset, height-offset);
            }
            else
            {
                float width  = _design_size[1] * _screen_size[0]/_screen_size[1];
                float offset = (width - _design_size[0])*0.5f;
                _ortho = make_ortho(-offset, width-offset, 0.f, _design_size[1]);
            }
            break;
        }

        case ResolutionResolveMode::SCALE_EXACT_FIT:
        {
            _resolved_size = _design_size;
            _ortho = make_ortho(0.f, _design_size[0], 0.f, _design_size[1]);
            break;
        }

        case ResolutionResolveMode::REFINE_FIXED_HEIGHT:
        {
            _resolved_size = {_screen_size[0], _design_size[1]};
            _ortho = make_ortho(0.f, _screen_size[0], 0.f, _design_size[1]);
            break;
        }

        case ResolutionResolveMode::REFINE_FIXED_WIDTH:
        {
            _resolved_size = {_design_size[0], _screen_size[1]};
            _ortho = make_ortho(0.f, _design_size[0], 0.f, _screen_size[1]);
            break;
        }

        case ResolutionResolveMode::REFINE_ALL:
        {
            _resolved_size = {_screen_size[0], _screen_size[1]};
            _ortho = make_ortho(0.f, _screen_size[0], 0.f, _screen_size[1]);
            break;
        }

        default:
            ASSERT( false, "undefined resolution resolved mode." );
    }
}

void CanvasDirector::resize(Transform& transform)
{
    Rect2f bounds({0, 0}, _resolved_size);
    resize_recursive(transform, bounds);
}

void CanvasDirector::resize_recursive(Transform& transform, const Rect2f& bounds)
{
    if( transform.has_component<CanvasDirector>() && transform.get_object() != get_object() )
        return;

    auto next_bounds = bounds;
    if( transform.has_component<Widget>() )
    {
        auto widget = transform.get_component<Widget>();
        widget->perform_resize(bounds);
        next_bounds = widget->get_bounds();
    }

    if( transform.has_component<Container::Trait>() )
    {
        (*transform.get_component<Container::Trait>())->on_format(transform, next_bounds);
    }
    else
    {
        transform.get_children().visit([&](Transform& ct)
        {
            resize_recursive(ct, next_bounds);
        });
    }
}

void CanvasSystem::on_spawn(SystemManager& manager)
{
    SystemWithEntities<CanvasDirector>::on_spawn(manager);
    _canvas.reset(Canvas::create());
    manager.get_dispatcher().subscribe<EvtInputMouse>(*this);
    manager.get_dispatcher().subscribe<EvtInputMousePosition>(*this);

}

void CanvasSystem::on_dispose(SystemManager& manager)
{
    SystemWithEntities<CanvasDirector>::on_dispose(manager);
    _canvas.reset();
    manager.get_dispatcher().unsubscribe<EvtInputMouse>(*this);
    manager.get_dispatcher().unsubscribe<EvtInputMousePosition>(*this);
}

void CanvasSystem::receive(const EvtInputMouse& evt)
{
    if( static_cast<size_t>(evt.button) >= kMaxMouseButton )
        return;

    auto& state = _mouse_states[static_cast<size_t>(evt.button)];
    if( ButtonAction::PRESS == evt.action )
    {
        for( auto pair : _entities )
        {
            EvtMousePress mouse_down;
            mouse_down.button   = evt.button;
            mouse_down.position = state.current * pair.second->get_resolved_size();

            if( _world.has_component<Transform>(pair.first) )
            {
                auto cps = _world.get_components<Transform, Widget, EventListenerGroup>(pair.first);
                if( std::get<0>(cps) != nullptr && std::get<1>(cps) != nullptr && std::get<2>(cps) != nullptr )
                    if( std::get<1>(cps)->is_inside(mouse_down.position, TransformSpace::WORLD) )
                        std::get<2>(cps)->emit(mouse_down);

                if( mouse_down.is_consumed() )
                {
                    set_mouse_focus(evt.button, pair.first);
                    return;
                }

                auto v = std::get<0>(cps)->get_children_with<Widget, EventListenerGroup>(true);
                for( auto iter = v.begin(); iter != v.end(); ++ iter )
                {
                    if( (*iter).get_component<Widget>()->is_inside(mouse_down.position, TransformSpace::WORLD) )
                        (*iter).get_component<EventListenerGroup>()->emit(mouse_down);
                    if( mouse_down.is_consumed() )
                    {
                        set_mouse_focus(evt.button, pair.first);
                        return;
                    }
                }
            }
        }
        return;
    }

    if( ButtonAction::RELEASE == evt.action )
    {
        auto object = state.object;
        state.object.invalidate();

        if( !_world.is_alive(object) || !_world.has_component<EventListenerGroup>(object) )
            return;

        if( _world.has_component<Transform>(object) && _world.has_component<Widget>(object) )
        {
            auto cps = _world.get_components<Transform, Widget, EventListenerGroup>(object);
            for( auto pair : _entities )
            {
                if( pair.second->has_component<Transform>() &&
                    (*std::get<0>(cps) == *pair.second->get_component<Transform>() ||
                     std::get<0>(cps)->is_ancestor(*pair.second->get_component<Transform>())) )
                {
                    auto current = state.current * pair.second->get_resolved_size();
                    auto start   = state.start * pair.second->get_resolved_size();
                    if( std::get<1>(cps)->is_inside(current, TransformSpace::WORLD) )
                    {
                        EvtMouseRelease mouse_release;
                        mouse_release.button    = evt.button;
                        mouse_release.position  = current;
                        mouse_release.pressed   = state.pressed;
                        mouse_release.delta     = current - start;
                        std::get<2>(cps)->emit(mouse_release);
                        return;
                    }
                    break;
                }
            }
        }

        EvtMouseLostFocus mouse_lost;
        mouse_lost.button = evt.button;
        _world.get_component<EventListenerGroup>(object)->emit(mouse_lost);
    }
}

void CanvasSystem::receive(const EvtInputMousePosition& evt)
{
    auto position = screen_to_design(evt.position);
    for( auto i = 0; i < kMaxMouseButton; i++ )
        _mouse_states[i].current = position;
}

void CanvasSystem::set_mouse_focus(MouseButton button, Entity object)
{
    if( static_cast<size_t>(button) >= kMaxMouseButton )
        return;

    auto& state = _mouse_states[static_cast<size_t>(button)];
    state.start = state.last = state.current;
    state.pressed = 0.f;

    if( _world.is_alive(state.object) && _world.has_component<EventListenerGroup>(state.object) )
    {
        EvtMouseLostFocus mouse_lost;
        _world.get_component<EventListenerGroup>(state.object)->emit(mouse_lost);
    }

    state.object = object;
}

Vector2f CanvasSystem::screen_to_design(const Vector2f& screen)
{
    return { screen[0] / _screen_size[0], (_screen_size[1] - screen[1]) / _screen_size[1] };
}

void CanvasSystem::set_screen_size(const Vector2f& size)
{
    _screen_size = size;
    for( auto pair : _entities )
    {
        pair.second->set_screen_size(size);
        if( _world.has_component<Transform>(pair.first) )
            pair.second->resize(*_world.get_component<Transform>(pair.first));
    }
}

void CanvasSystem::update(float dt)
{
    for( auto pair : _entities )
    {
        if( _world.has_component<View::Trait>(pair.first) )
            (*_world.get_component<View::Trait>(pair.first))->on_update(dt);

        if( _world.has_component<Transform>(pair.first) )
        {
            auto transform = _world.get_component<Transform>(pair.first);
            transform->get_children_with<View::Trait>(true).visit([&](Transform& ct, View::Trait& cv)
            {
                cv->on_update(dt);
            });
        }
    }

    for( auto i = 0; i < kMaxMouseButton; i++ )
    {
        auto& state = _mouse_states[i];
        if( !_world.is_alive(state.object) || !_world.has_component<EventListenerGroup>(state.object) )
            return;

        state.pressed += dt;
        auto listeners = _world.get_component<EventListenerGroup>(state.object);

        if( _world.has_component<Transform>(state.object) && _world.has_component<Widget>(state.object) )
        {
            auto cps = _world.get_components<Transform, Widget>(state.object);
            for( auto pair : _entities )
            {
                if( pair.second->has_component<Transform>() &&
                    (*std::get<0>(cps) == *pair.second->get_component<Transform>() ||
                     std::get<0>(cps)->is_ancestor(*pair.second->get_component<Transform>())) )
                {
                    auto current = state.current * pair.second->get_resolved_size();
                    auto last = state.last * pair.second->get_resolved_size();
                    if( !std::get<1>(cps)->is_inside(current, TransformSpace::WORLD) )
                    {
                        EvtMouseLostFocus mouse_lost;
                        mouse_lost.button = static_cast<MouseButton>(i);

                        listeners->emit(mouse_lost);
                        state.object.invalidate();
                        break;
                    }

                    if( (current-last).length_square() < kMoveThreshold )
                    {
                        EvtMouseMove mouse_move;
                        mouse_move.button    = static_cast<MouseButton>(i);
                        mouse_move.position  = current;
                        mouse_move.pressed   = state.pressed;
                        mouse_move.delta     = current - last;

                        listeners->emit(mouse_move);
                        state.last = state.current;
                        break;
                    }
                }
            }
        }
        else
        {
            EvtMouseLostFocus mouse_lost;
            mouse_lost.button = static_cast<MouseButton>(i);

            listeners->emit(mouse_lost);
            state.object.invalidate();
        }
    }
}

void CanvasSystem::draw()
{
    for( auto pair : _entities )
    {
        _canvas->begin_frame(pair.second->get_ortho());
        if( _world.has_component<View::Trait>(pair.first) && _world.has_component<Widget>(pair.first) )
        {
            auto widget = _world.get_component<Widget>(pair.first);
            (*_world.get_component<View::Trait>(pair.first))->on_draw(*_canvas, widget->get_bounds());
        }

        if( _world.has_component<Transform>(pair.first) )
        {
            auto transform = _world.get_component<Transform>(pair.first);
            transform->get_children_with<Widget, View::Trait>(true).visit(
                [&](Transform& ct, Widget& cw, View::Trait& cv)
                {
                    cv->on_draw(*_canvas, cw.get_bounds());
                });
        }
        _canvas->end_frame();
    }
}

NS_FLOW2D_UI_END