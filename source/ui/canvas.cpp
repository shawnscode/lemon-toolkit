// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#include <ui/canvas.hpp>
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
            _resolved_size = _resolved_size;
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
    Rect2f bounds = { 0, 0, _resolved_size[0], _resolved_size[1] };
    transform.get_children().visit([&](Transform& ct) { resize_recursive(ct, bounds); });
}

void CanvasDirector::resize_recursive(Transform& transform, const Rect2f& bounds)
{
    if( transform.has_component<CanvasDirector>() )
        return;

    auto next_bounds = bounds;
    if( transform.has_component<Widget>() )
    {
        auto widget = transform.get_component<Widget>();
        widget->on_resize(bounds);
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

CanvasSystem::CanvasSystem(EntityManager& w)
: _world(w)
{
    _canvas.reset(Canvas::create());
    _world.get_dispatcher().subscribe<EvtComponentAdded<CanvasDirector>>(*this);
    _world.get_dispatcher().subscribe<EvtComponentRemoved<CanvasDirector>>(*this);
}

CanvasSystem::~CanvasSystem()
{
    _world.get_dispatcher().unsubscribe<EvtComponentAdded<CanvasDirector>>(*this);
    _world.get_dispatcher().unsubscribe<EvtComponentRemoved<CanvasDirector>>(*this);
}

void CanvasSystem::receive(const EvtComponentAdded<CanvasDirector>& evt)
{
    evt.component.set_screen_size(_screen_size);
    _scalers[evt.entity] = &evt.component;
}

void CanvasSystem::receive(const EvtComponentRemoved<CanvasDirector>& evt)
{
    _scalers.erase(evt.entity);
}

void CanvasSystem::set_screen_size(const Vector2f& size)
{
    _screen_size = size;
    for( auto pair : _scalers )
    {
        pair.second->set_screen_size(size);
        if( _world.has_component<Transform>(pair.first) )
            pair.second->resize(*_world.get_component<Transform>(pair.first));
    }
}

void CanvasSystem::update(float dt)
{
    for( auto pair : _scalers )
    {
        if( _world.has_component<Transform>(pair.first) )
        {
            auto transform = _world.get_component<Transform>(pair.first);
            if( transform->has_component<View::Trait>() )
                (*transform->get_component<View::Trait>())->on_update(dt);
            
            transform->get_children_with<View::Trait>().visit([&](Transform& ct, View::Trait& cv)
            {
                cv->on_update(dt);
            });
        }
    }
}

void CanvasSystem::draw()
{
    for( auto pair : _scalers )
    {
        _canvas->begin_frame(pair.second->get_ortho());
        if( _world.has_component<Transform>(pair.first) )
        {
            auto transform = _world.get_component<Transform>(pair.first);
            if( transform->has_component<View::Trait>() && transform->has_component<Widget>() )
            {
                auto widget = transform->get_component<Widget>();
                (*transform->get_component<View::Trait>())->on_draw(*_canvas, widget->get_bounds());
            }

            transform->get_children_with<Widget, View::Trait>().visit(
                [&](Transform& ct, Widget& cw, View::Trait& cv)
                {
                    cv->on_draw(*_canvas, cw.get_bounds());
                });
        }
        _canvas->end_frame();
    }
}

NS_FLOW2D_UI_END