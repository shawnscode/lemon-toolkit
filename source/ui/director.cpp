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
        next_bounds = widget->get_bounds_in();
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

void CanvasSystem::find_mouse_focus(Entity director, EvtMousePress& evt, Transform& transform)
{
    auto& state = _mouse_states[static_cast<size_t>(evt.button)];
    state.director = director;
    state.start = state.last = state.current = evt.position;
    state.pressed = 0.f;
    state.objects.push_back(transform.get_object());

    auto v = transform.get_children_with<Widget, EventListenerGroup>(true);
    for( auto iter = v.begin(); iter != v.end(); ++iter )
    {
        if( (*iter).get_component<Widget>()->is_inside(evt.position, TransformSpace::WORLD) )
            (*iter).get_component<EventListenerGroup>()->emit(evt);

        if( evt.has_focus() )
            state.objects.push_back((*iter).get_object());
    }
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
            mouse_down.position = screen_to_design(evt.position) * pair.second->get_resolved_size();

            // find first Entity that consumed Press action
            if( _world.has_component<Transform>(pair.first) )
            {
                auto cps = _world.get_components<Transform, Widget, EventListenerGroup>(pair.first);
                if( std::get<0>(cps) != nullptr && std::get<1>(cps) != nullptr && std::get<2>(cps) != nullptr )
                    if( std::get<1>(cps)->is_inside(mouse_down.position, TransformSpace::WORLD) )
                        std::get<2>(cps)->emit(mouse_down);

                if( mouse_down.has_focus() )
                {
                    find_mouse_focus(pair.first, mouse_down, *std::get<0>(cps));
                    return;
                }

                auto v = std::get<0>(cps)->get_children_with<Widget, EventListenerGroup>(true);
                for( auto iter = v.begin(); iter != v.end(); ++ iter )
                {
                    if( (*iter).get_component<Widget>()->is_inside(mouse_down.position, TransformSpace::WORLD) )
                        (*iter).get_component<EventListenerGroup>()->emit(mouse_down);
                    if( mouse_down.has_focus() )
                    {
                        find_mouse_focus(pair.first, mouse_down, *iter);
                        return;
                    }
                }
            }
        }
        return;
    }

    if( ButtonAction::RELEASE == evt.action )
    {
        if( state.objects.size() == 0 )
            return;

        EvtMouseRelease mouse_release;
        mouse_release.button    = evt.button;
        mouse_release.position  = state.current;
        mouse_release.pressed   = state.pressed;
        mouse_release.delta     = state.current - state.start;

        for( auto object : state.objects )
        {
            if( !_world.has_component<EventListenerGroup>(object) )
                continue;

            _world.get_component<EventListenerGroup>(object)->emit(mouse_release);
        }

        state.objects.clear();
        return;
    }
}

void CanvasSystem::receive(const EvtInputMousePosition& evt)
{
    auto position = screen_to_design(evt.position);
    for( auto i = 0; i < kMaxMouseButton; i++ )
    {
        auto& state = _mouse_states[i];
        if( _world.has_component<CanvasDirector>(state.director) )
        {
            auto director = _world.get_component<CanvasDirector>(state.director);
            state.current = position * director->get_resolved_size();
        }
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
        if( state.objects.size() == 0 )
            continue;

        EvtMouseLostFocus mouse_lost;
        mouse_lost.button   = static_cast<MouseButton>(i);
        mouse_lost.position = state.current;
        mouse_lost.pressed  = state.pressed;
        mouse_lost.delta    = state.current - state.start;

        // disable focus to all the tracking objects if we lost director
        if( !_world.has_component<CanvasDirector>(state.director) )
        {
            for( auto object : state.objects )
            {
                if( _world.has_component<EventListenerGroup>(object) )
                    _world.get_component<EventListenerGroup>(object)->emit(mouse_lost);
            }

            state.objects.clear();
            continue;
        }

        state.pressed += dt;
        if( (state.current-state.last).length_square() < kMoveThreshold )
            continue;

        EvtMouseMove mouse_move;
        mouse_move.button   = static_cast<MouseButton>(i);
        mouse_move.position = state.current;
        mouse_move.pressed  = state.pressed;
        mouse_move.delta    = state.current - state.last;

        Entity focus;
        for( int32_t i = state.objects.size()-1; i>=0; i --)
        {
            auto object = state.objects[i];
            if( !_world.has_components<Widget, EventListenerGroup>(object) )
            {
                state.objects.erase(state.objects.begin()+i);
                continue;
            }

            auto cps = _world.get_components<Widget, EventListenerGroup>(object);
            if( !std::get<0>(cps)->is_inside(state.current, TransformSpace::WORLD) )
            {
                std::get<1>(cps)->emit(mouse_lost);
                state.objects.erase(state.objects.begin()+i);
                continue;
            }

            std::get<1>(cps)->emit(mouse_move);
            if( mouse_move.has_focus() )
            {
                focus = object;
                break;;
            }
        }

        if( mouse_move.has_focus() )
        {
            for( auto object : state.objects )
            {
                if( object != focus )
                    _world.get_component<EventListenerGroup>(object)->emit(mouse_lost);
            }

            state.objects.clear();
            state.objects.push_back(focus);
        }

        state.last = state.current;
    }
}

Vector2f CanvasSystem::screen_to_design(const Vector2f& screen)
{
    return { screen[0] / _screen_size[0], (_screen_size[1] - screen[1]) / _screen_size[1] };
}

void CanvasSystem::set_screen_size(const Vector2f& size)
{
    if( equals(_screen_size, size) )
        return;

    _screen_size = size;

    for( auto pair : _entities )
    {
        pair.second->set_screen_size(size);
        if( _world.has_component<Transform>(pair.first) )
            pair.second->resize(*_world.get_component<Transform>(pair.first));
    }
}

void CanvasSystem::draw()
{
    for( auto pair : _entities )
    {
        _canvas->begin_frame(pair.second->get_ortho());
        if( _world.has_component<View::Trait>(pair.first) )
            (*_world.get_component<View::Trait>(pair.first))->on_draw(*_canvas);

        if( _world.has_component<Transform>(pair.first) )
        {
            auto transform = _world.get_component<Transform>(pair.first);
            transform->get_children_with<View::Trait>(true).visit([&](Transform& ct, View::Trait& cv)
            {
                cv->on_draw(*_canvas);
            });
        }
        _canvas->end_frame();
    }
}

NS_FLOW2D_UI_END