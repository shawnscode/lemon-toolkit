// @date 2016/07/20
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/components.hpp>
#include <core/system.hpp>
#include <scene/transform.hpp>
#include <math/vector.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

// const static size_t kMaxSizeofAction = 32;
//
struct ActionExecutor : ComponentWithEnvironment<>
{
    ~ActionExecutor();

    void    run(Action*, bool override = false);
    void    stop();
    void    update(float);
    bool    is_finished() const;

protected:
    friend class ActionSystem;
    Action*         _task   = nullptr;
    bool            _paused = false;
};

struct Action
{
    virtual ~Action() {}

    virtual void on_spawn(ActionExecutor&) {}

    virtual void reset() = 0;
    virtual void update(float) = 0;
    virtual bool is_finished() const = 0;

    // placement new/delete
    void* operator new(size_t, MemoryChunks*);
    void  operator delete(void*);

protected:
    // actions' memory is always managed by specified object pool
    void* operator new(size_t);
    void* operator new[](size_t);
    void operator delete[](void*);

    Action() {};
    Action(const Action&) = delete;
    Action& operator = (const Action&) = delete;

// static methods
public:
    template<typename T> static size_t get_size_of()
    {
        auto found = memories.find(sizeof(T));
        if( found == memories.end() )
            return 0;
        return memories[sizeof(T)]->size();
    }

    template<typename T, typename ... Args> static T* spawn(Args&& ... args)
    {
        static_assert( std::is_base_of<Action, T>::value, "T is not action." );
        auto found = memories.find(sizeof(T));
        if( found == memories.end() )
            memories.insert( std::make_pair(sizeof(T), new MemoryChunks(sizeof(T) + sizeof(MemoryChunks*), 64)) );
        return new (memories[sizeof(T)]) T(std::forward<Args>(args)...);
    }

protected:
    static std::unordered_map<size_t, MemoryChunks*> memories;
};

///
enum class EaseTween
{
    LINEAR,

    QUADRATIC_IN,
    QUADRATIC_OUT,

    CUBIC_IN,
    CUBIC_OUT,

    QUARTIC_IN,
    QUARTIC_OUT,

    SINE_IN,
    SINE_OUT,

    EXPONENTIAL_IN,
    EXPONENTIAL_OUT,

    CIRCULAR_IN,
    CIRCULAR_OUT,

    BOUNCE_IN,
    BOUNCE_OUT
};

struct ActionFiniteTime : public Action
{
    ActionFiniteTime(float duration)
    : _duration(duration)
    {
        with_tween(EaseTween::LINEAR);
    }

    virtual ~ActionFiniteTime() {}

    void reset() final { _eplased = 0.f; }
    void update(float dt) final
    {
        _eplased += dt;
        step(_tweener(std::min(_eplased/_duration, 1.0f), _tween_params));
    }
    bool is_finished() const final { return _eplased >= _duration; }

    float get_duration() const { return _duration; }
    float get_eplased() const { return _eplased; }
    ActionFiniteTime* with_tween(EaseTween, size_t c = 0, float* params = nullptr);

    virtual void step(float) = 0;

protected:
    using tweener = std::function<float(float, float*)>;

    float   _duration = 0.f, _eplased = 0.f;
    tweener _tweener;
    float   _tween_params[4];
};

/// TRANSFORMATION TASKS
struct ActionTransform : public ActionFiniteTime
{
    ActionTransform(float d, TransformSpace space) : ActionFiniteTime(d), _space(space) {}
    virtual void on_spawn(ActionExecutor&) override;

protected:
    Transform*      _transform = nullptr;
    TransformSpace  _space = TransformSpace::SELF;
};

struct ActionMoveTo : public ActionTransform
{
    ActionMoveTo(float d, const Vector2f& p, TransformSpace space = TransformSpace::SELF)
    : ActionTransform(d, space), _to(p) {}

    void on_spawn(ActionExecutor& executor) override
    {
        ActionTransform::on_spawn(executor);
        _from = _transform->get_position(_space);
    }

    void step(float t) override
    {
        _transform->set_position(_from+t*(_to-_from), _space);
    }

protected:
    Vector2f _from, _to;
};

struct ActionMoveBy : public ActionTransform
{
    ActionMoveBy(float d, const Vector2f& p, TransformSpace space = TransformSpace::SELF)
    : ActionTransform(d, space), _to(p) {}

    void on_spawn(ActionExecutor& executor) override
    {
        ActionTransform::on_spawn(executor);
        _from = _transform->get_position(_space);
    }

    void step(float t) override
    {
        _transform->set_position(_from + t*_to, _space);
    }

protected:
    Vector2f _from, _to;
};

/// decorator actions
struct ActionRepeat : public Action
{
    ActionRepeat(Action* a, size_t c) : _slave(a), _times(c) {}

    void reset() override
    {
        _slave->reset();
        _count = 0;
    }

    void update(float dt) override
    {
        if( !_slave->is_finished() )
            _slave->update(dt);

        while( _slave->is_finished() && _count < _times )
        {
            _slave->reset();
            _count ++;
        }
    }

    bool is_finished() const override
    {
        return _times == 0 || (_count >= _times && _slave->is_finished());
    }

protected:
    Action* _slave = nullptr;
    size_t  _count = 0;
    size_t  _times = 0;
};

/// compositable actions
struct ActionCompositor : public Action
{
    ActionCompositor(std::initializer_list<Action*> values)
    {
        for( auto value : values ) _actions.push_back(value);
    }

    virtual ~ActionCompositor()
    {
        for( auto value : _actions ) delete value;
    }

    virtual void reset() override
    {
        for( auto value : _actions ) value->reset();
    }

protected:
    std::vector<Action*> _actions;
};

struct ActionSequence : public ActionCompositor
{
    ActionSequence(std::initializer_list<Action*> actions) : ActionCompositor(actions) {}

    void reset() override
    {
        ActionCompositor::reset();
        _current = 0;
    }

    void update(float dt) override
    {
        do
        {
            if( !_actions[_current]->is_finished() )
            {
                _actions[_current]->update(dt);
                break;
            }
            else _current ++;
        } while( !is_finished() );
    }

    bool is_finished() const override
    {
        return _current >= _actions.size();
    }

protected:
    size_t _current = 0;
};

struct ActionParallel : public ActionCompositor
{
    ActionParallel(std::initializer_list<Action*> actions) : ActionCompositor(actions) {}

    void reset() override
    {
        ActionCompositor::reset();
        _finished = false;
    }

    void update(float dt) override
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

    bool is_finished() const override
    {
        return _finished;
    }

protected:
    bool _finished = false;
};

/// misc
struct ActionWaitForSeconds : public ActionFiniteTime
{
    ActionWaitForSeconds(float t) : ActionFiniteTime(t) {}
};

struct ActionClosure : Action
{
    using closure = std::function<bool()>;
    ActionClosure(const closure& c) : _closure(c) {}

    void reset() override { _break = false; }
    void update(float dt) override { _break = _closure(); };
    bool is_finished() const override { return _closure && !_break; }

protected:
    bool    _break      = false;
    closure _closure    = nullptr;
};

///
struct ActionSystem : SystemWithEntities<ActionExecutor>
{
    ActionSystem(EntityManager& world) : SystemWithEntities(world) {}
    void update(float) override;
};

NS_FLOW2D_END