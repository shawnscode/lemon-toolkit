// @date 2016/07/20
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/action.hpp>

NS_FLOW2D_BEGIN

ActionExecutor::~ActionExecutor()
{
    if( _task != nullptr )
    {
        delete _task;
        _task = nullptr;
    }
}

void ActionExecutor::run(Action* task, bool override)
{
    if( _task != nullptr )
    {
        if( !override && !_task->is_finished() )
        {
            LOGW("duplicated action task.");
            return;
        }

        delete _task;
    }

    _task = task;
    _task->on_spawn(*this);
}

void ActionExecutor::stop()
{
    if( _task != nullptr )
    {
        delete _task;
        _task = nullptr;
    }
}

void ActionExecutor::update(float dt)
{
    if( _task != nullptr && !_task->is_finished() )
        _task->update(dt);
}

bool ActionExecutor::is_finished() const
{
    return _task == nullptr ? true : _task->is_finished();
}

std::unordered_map<size_t, MemoryChunks*> Action::memories;

void* Action::operator new(size_t size, MemoryChunks* chunks)
{
    ENSURE( chunks->element_size() == size + sizeof(MemoryChunks*) );
    auto ptrs = chunks->malloc();
    *static_cast<MemoryChunks**>(ptrs) = chunks;
    return static_cast<void*>((static_cast<uint8_t*>(ptrs)+sizeof(MemoryChunks*)));
}

void Action::operator delete(void* v)
{
    v = static_cast<void*>(static_cast<uint8_t*>(v)-sizeof(MemoryChunks*));
    MemoryChunks* p = *static_cast<MemoryChunks**>(v);
    p->free(v);
}

ActionFiniteTime::ActionFiniteTime(float duration)
: _duration(duration)
{
    with_tween(EaseTween::LINEAR);
}

void ActionFiniteTime::reset()
{
    _eplased = 0;
}

void ActionFiniteTime::update(float dt)
{
    _eplased += dt;
    step(_tweener(std::min(_eplased/_duration, 1.0f)));
}

bool ActionFiniteTime::is_finished() const
{
    return _eplased >= _duration;
}


static float tween_linear(float t) { return t; }

static float tween_sine_in(float t) { return -1 * std::cos(t * math::pi_div_2) + 1; }
static float tween_sine_out(float t) { return std::sin(t * math::pi_div_2); }

static float tween_quad_in(float t) { return t * t; }
static float tween_quad_out(float t) { return -1 * t * ( t - 2 ); }

static float tween_cubic_in(float t) { return t * t * t; }
static float tween_cubic_out(float t) { t -= 1; return (t * t * t + 1); }

static float tween_quart_in(float t) { return t * t * t * t; }
static float tween_quart_out(float t) { t -= 1; return -(t * t * t * t - 1); }

static float tween_expo_in(float t) { return t == 0 ? 0 : std::powf(2, 10 * (t/1 - 1)) - 1 * 0.001f; }
static float tween_expo_out(float t) { return t == 1 ? 1 : (-std::powf(2, -10 * t / 1) + 1); }

static float tween_circ_in(float t) { return -1 * (std::sqrt(1 - t * t) - 1); }
static float tween_circ_out(float t) { t = t - 1; return sqrt(1 - t * t); }

static float tween_bounce_out(float t)
{
    if (t < 1 / 2.75f)
    {
        return 7.5625f * t * t;
    }
    else if (t < 2 / 2.75f)
    {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    }
    else if(t < 2.5f / 2.75f)
    {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    }

    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
}

static float tween_bounce_in(float t) { return 1 - tween_bounce_out(1 - t); }

#define CASE(t, func) case t: { _tweener = func; break; }

ActionFiniteTime* ActionFiniteTime::with_tween(EaseTween type)
{
    switch(type)
    {
        CASE(EaseTween::LINEAR,         tween_linear)
        CASE(EaseTween::QUADRATIC_IN,   tween_quad_in)
        CASE(EaseTween::QUADRATIC_OUT,  tween_quad_out)
        CASE(EaseTween::CUBIC_IN,       tween_cubic_in)
        CASE(EaseTween::CUBIC_OUT,      tween_cubic_out)
        CASE(EaseTween::QUARTIC_IN,     tween_quart_in)
        CASE(EaseTween::QUARTIC_OUT,    tween_quart_out)
        CASE(EaseTween::SINE_IN,        tween_sine_in)
        CASE(EaseTween::SINE_OUT,       tween_sine_out)
        CASE(EaseTween::EXPONENTIAL_IN, tween_expo_in)
        CASE(EaseTween::EXPONENTIAL_OUT,tween_expo_out)
        CASE(EaseTween::CIRCULAR_IN,    tween_circ_in)
        CASE(EaseTween::CIRCULAR_OUT,   tween_circ_out)
        CASE(EaseTween::BOUNCE_IN,      tween_bounce_in)
        CASE(EaseTween::BOUNCE_OUT,     tween_bounce_out)

        default:
        {
            FATAL("not supported EaseTween.");
            break;
        }
    }

    return this;
}

ActionTransform::ActionTransform(float d, TransformSpace space)
: ActionFiniteTime(d), _space(space)
{}

void ActionTransform::on_spawn(ActionExecutor& executor)
{
    ASSERT( executor.has_component<Transform>(),
        "trying to perform transformation action on entity without Transform component." );
    _transform = executor.get_component<Transform>();
}

void ActionSystem::update(float dt)
{
    for( auto pair : _entities )
        pair.second->update(dt);
}

ActionMoveTo::ActionMoveTo(float d, const Vector2f& p, TransformSpace space)
: ActionTransform(d, space), _to(p)
{}

void ActionMoveTo::on_spawn(ActionExecutor& executor)
{
    ActionTransform::on_spawn(executor);
    _from = _transform->get_position(_space);
}

void ActionMoveTo::reset()
{
    ActionTransform::reset();
    _from = _transform->get_position(_space);
}

void ActionMoveTo::step(float t)
{
    _transform->set_position(_from+t*(_to-_from), _space);
}

ActionMoveBy::ActionMoveBy(float d, const Vector2f& p, TransformSpace space)
: ActionTransform(d, space), _to(p)
{}

void ActionMoveBy::on_spawn(ActionExecutor& executor)
{
    ActionTransform::on_spawn(executor);
    _from = _transform->get_position(_space);
}

void ActionMoveBy::reset()
{
    ActionTransform::reset();
    _from = _transform->get_position(_space);
}

void ActionMoveBy::step(float t)
{
    _transform->set_position(_from + t*_to, _space);
}

ActionRepeat::ActionRepeat(Action* a, size_t c)
:_slave(a), _times(c)
{}

void ActionRepeat::on_spawn(ActionExecutor& executor)
{
    _slave->on_spawn(executor);
}

void ActionRepeat::reset()
{
    _slave->reset();
    _count = 0;
}

void ActionRepeat::update(float dt)
{
    if( !_slave->is_finished() )
        _slave->update(dt);

    while( _slave->is_finished() && _count < _times )
    {
        _count ++;
        if( !is_finished() ) _slave->reset();
    }
}

bool ActionRepeat::is_finished() const
{
    return _times == 0 || (_count >= _times && _slave->is_finished());
}

void ActionSequence::reset()
{
    ActionCompositor::reset();
    _current = 0;
}

void ActionSequence::update(float dt)
{
    do
    {
        if( !_actions[_current]->is_finished() )
        {
            _actions[_current]->update(dt);
            if( _actions[_current]->is_finished() )
                _current ++;
            break;
        }
        else _current ++;
    } while( !is_finished() );
}

bool ActionSequence::is_finished() const
{
    return _current >= _actions.size();
}

void ActionParallel::reset()
{
    ActionCompositor::reset();
    _finished = false;
}

void ActionParallel::update(float dt)
{
    _finished = true;
    for( auto task : _actions )
    {
        if( !task->is_finished() )
        {
            _finished = false;
            task->update(dt);
        }
    }
}

bool ActionParallel::is_finished() const
{
    return _finished;
}

ActionWaitForSeconds::ActionWaitForSeconds(float t)
: ActionFiniteTime(t)
{}

ActionClosure::ActionClosure(const closure& c)
: _closure(c)
{}

void ActionClosure::reset()
{
    _break = false;
}

void ActionClosure::update(float dt)
{
    _break = _closure();
};

bool ActionClosure::is_finished() const
{
    return _closure && !_break;
}

NS_FLOW2D_END