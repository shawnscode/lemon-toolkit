// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/director.hpp>
#include <ui/listener.hpp>
#include <scene/transform.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_UI_BEGIN

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
    if( ButtonAction::PRESS == evt.action )
    {
        auto down = construct_event<EvtMouseDown>(evt);
        dispatch<EvtMouseDown>(down);
    }
    else if( ButtonAction::RELEASE == evt.action )
    {
        auto up = construct_event<EvtMouseUp>(evt);
        dispatch<EvtMouseUp>(up);
        auto click = construct_event<EvtMouseClick>(evt);
        dispatch<EvtMouseClick>(click);
    }
}

void CanvasSystem::receive(const EvtInputMousePosition& evt)
{
    Vector2f next   = { evt.position[0], _screen_size[1] - evt.position[1] };
    _mouse_delta    = _mouse_position - next;
    _mouse_position = clamp(
        Vector2f{next[0]/_screen_size[0], next[1]/_screen_size[1]}, {0.f, 0.f}, {1.f, 1.f});
}

template<typename T> T CanvasSystem::construct_event(const EvtInputMouse& evt)
{
    T event;
    event.button    = evt.button;
    event.position  = _mouse_position;
    event.delta     = _mouse_delta;
    event.pressed   = 0.f;
    return event;
}

template<typename T> void CanvasSystem::dispatch(T& evt)
{
    for( auto pair : _entities )
    {
        auto cevt = evt;
        cevt.position = _mouse_position * pair.second->get_resolved_size();

        if( _world.has_component<Transform>(pair.first) )
        {
            auto cps = _world.get_components<Transform, Widget, EventListenerGroup>(pair.first);
            if( std::get<0>(cps) != nullptr && std::get<1>(cps) != nullptr && std::get<2>(cps) != nullptr )
                if( std::get<1>(cps)->is_inside(cevt.position, TransformSpace::WORLD) )
                    std::get<2>(cps)->emit<T>(*std::get<0>(cps), cevt);

            if( cevt.is_consumed() )
                return;

            auto v = std::get<0>(cps)->get_children_with<Widget, EventListenerGroup>(true);
            for( auto iter = v.begin(); iter != v.end(); ++ iter )
            {
                if( (*iter).get_component<Widget>()->is_inside(cevt.position, TransformSpace::WORLD) )
                    (*iter).get_component<EventListenerGroup>()->emit<T>(*iter, cevt);
                if( cevt.is_consumed() )
                    return;
            }
        }
    }
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