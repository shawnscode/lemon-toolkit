// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/ui/canvas.hpp>
#include <graphic/canvas.hpp>

NS_FLOW2D_BEGIN

CanvasScaler::CanvasScaler()
{
    _ortho = resolve();
}

void CanvasScaler::set_resolve_mode(ResolutionResolveMode m)
{
    _resolve_mode = m;
    _ortho = resolve();
}

void CanvasScaler::set_screen_size(const Vector2f& size)
{
    _screen_size = max(size, {1.f, 1.f});
    _ortho = resolve();
}

void CanvasScaler::set_design_size(const Vector2f& size)
{
    _design_size = max(size, {1.f, 1.f});
    _ortho = resolve();
}

Matrix3f CanvasScaler::get_ortho() const
{
    return _ortho;
}

Matrix3f CanvasScaler::resolve() const
{
    switch( _resolve_mode )
    {
        case ResolutionResolveMode::SCALE_NO_BORDER:
        {
            if( _screen_size[0]/_screen_size[1] < _design_size[0]/_design_size[1] )
            {
                float width = _design_size[1] * _screen_size[0] / _screen_size[1];
                float offset = (_design_size[0] - width) * 0.5f;
                return make_ortho(offset, width+offset, 0.f, _design_size[1]);
            }
            else
            {
                float height = _design_size[0] * _screen_size[1] / _screen_size[0];
                float offset = (_design_size[1] - height) * 0.5f;
                return make_ortho(0.f, _design_size[0], offset, height+offset);
            }
        }

        case ResolutionResolveMode::SCALE_SHOW_ALL:
        {
            if( _screen_size[0]/_screen_size[1] < _design_size[0]/_design_size[1] )
            {
                float height = _design_size[0] * _screen_size[1]/_screen_size[0];
                float offset = (height - _design_size[1])*0.5f;
                return make_ortho(0.f, _design_size[0], -offset, height-offset);
            }
            else
            {
                float width  = _design_size[1] * _screen_size[0]/_screen_size[1];
                float offset = (width - _design_size[0])*0.5f;
                return make_ortho(-offset, width-offset, 0.f, _design_size[1]);
            }
        }

        case ResolutionResolveMode::SCALE_EXACT_FIT:
            return make_ortho(0.f, _design_size[0], 0.f, _design_size[1]);

        case ResolutionResolveMode::REFINE_FIXED_HEIGHT:
            return make_ortho(0.f, _screen_size[0], 0.f, _design_size[1]);

        case ResolutionResolveMode::REFINE_FIXED_WIDTH:
            return make_ortho(0.f, _design_size[0], 0.f, _screen_size[1]);

        case ResolutionResolveMode::REFINE_ALL:
            return make_ortho(0.f, _screen_size[0], 0.f, _screen_size[1]);

        default:
        {
            ASSERT( false, "undefined resolution resolved mode." );
            return {0.f, 0.f};
        }
    }
}

CanvasSystem::CanvasSystem(EntityManager& w)
: _world(w)
{
    _canvas.reset(Canvas::create());
    _world.get_dispatcher().subscribe<EvtComponentAdded<CanvasScaler>>(*this);
    _world.get_dispatcher().subscribe<EvtComponentRemoved<CanvasScaler>>(*this);
}

CanvasSystem::~CanvasSystem()
{
    _world.get_dispatcher().unsubscribe<EvtComponentAdded<CanvasScaler>>(*this);
    _world.get_dispatcher().unsubscribe<EvtComponentRemoved<CanvasScaler>>(*this);
}

void CanvasSystem::receive(const EvtComponentAdded<CanvasScaler>& evt)
{
    evt.component.set_screen_size(_screen_size);
    _scalers[evt.entity] = &evt.component;
}

void CanvasSystem::receive(const EvtComponentRemoved<CanvasScaler>& evt)
{
    _scalers.erase(evt.entity);
}

void CanvasSystem::set_screen_size(const Vector2f& size)
{
    _screen_size = size;
    for( auto pair : _scalers )
        pair.second->set_screen_size(size);
}

void CanvasSystem::update(float dt)
{
    for( auto pair : _scalers )
    {
        if( _world.has_component<UIElement>(pair.first) )
        {
            _world.get_component<UIElement>(pair.first)->update(dt);
            _world.get_component<UIElement>(pair.first)->rearrange();
        }
    }
}

void CanvasSystem::draw()
{
    for( auto pair : _scalers )
    {
        _canvas->begin_frame(pair.second->get_ortho());
        if( _world.has_component<UIElement>(pair.first) )
        {
            _world.get_component<UIElement>(pair.first)->draw(*_canvas);
        }
        _canvas->end_frame();
    }
}

NS_FLOW2D_END