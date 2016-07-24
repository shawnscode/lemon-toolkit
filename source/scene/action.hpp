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

protected:
    template<typename T, typename ... Args> static T* spawn(Args&& ... args)
    {
        static_assert( std::is_base_of<Action, T>::value, "T is not action." );
        auto found = memories.find(sizeof(T));
        if( found == memories.end() )
            memories.insert( std::make_pair(sizeof(T), new MemoryChunks(sizeof(T) + sizeof(MemoryChunks*), 64)) );
        return new (memories[sizeof(T)]) T(std::forward<Args>(args)...);
    }

    static std::unordered_map<size_t, MemoryChunks*> memories;
};

#define STATIC_SPAWN(T) template<typename ... Args> static T* spawn(Args&& ... args) \
    { return Action::spawn<T>(std::forward<Args>(args)...); }

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
    ActionFiniteTime(float);
    virtual ~ActionFiniteTime() {}

    virtual void reset() override;
    virtual void update(float) override;
    virtual bool is_finished() const override;
    virtual void step(float) = 0;

    float get_duration() const { return _duration; }
    float get_eplased() const { return _eplased; }
    ActionFiniteTime* with_tween(EaseTween);

protected:
    using tweener = std::function<float(float)>;

    float   _duration = 0.f, _eplased = 0.f;
    tweener _tweener;
};

/// TRANSFORMATION TASKS
struct ActionTransform : public ActionFiniteTime
{
    ActionTransform(float, TransformSpace);
    virtual void on_spawn(ActionExecutor&) override;

protected:
    Transform*      _transform = nullptr;
    TransformSpace  _space = TransformSpace::SELF;
};

struct ActionMoveTo : public ActionTransform
{
    STATIC_SPAWN(ActionMoveTo);
    ActionMoveTo(float, const Vector2f&, TransformSpace space = TransformSpace::SELF);

    virtual void on_spawn(ActionExecutor& executor) override;
    virtual void reset() override;
    virtual void step(float t) override;

protected:
    Vector2f _from, _to;
};

struct ActionMoveBy : public ActionTransform
{
    STATIC_SPAWN(ActionMoveBy);
    ActionMoveBy(float, const Vector2f&, TransformSpace space = TransformSpace::SELF);

    virtual void on_spawn(ActionExecutor& executor) override;
    virtual void reset() override;
    virtual void step(float t) override;

protected:
    Vector2f _from, _to;
};

/// decorator actions
struct ActionRepeat : public Action
{
    STATIC_SPAWN(ActionRepeat);
    ActionRepeat(Action*, size_t);

    virtual void on_spawn(ActionExecutor& executor) override;
    virtual void reset() override;
    virtual void update(float dt) override;
    virtual bool is_finished() const override;

protected:
    Action* _slave = nullptr;
    size_t  _count = 0;
    size_t  _times = 0;
};

/// compositable actions
template<typename T> struct ActionCompositor : public Action
{
    template<typename ... Args> static T* spawn(Args&& ... args)
    {
        auto compositor = Action::spawn<T>();
        return compositor->insert(std::forward<Args>(args) ...);
    }

    template<typename A1, typename A2, typename ... Args> T* insert(A1* a1, A2* a2, Args&& ... args)
    {
        _actions.push_back(a1);
        return insert(a2, std::forward<Args>(args) ...);
    }

    template<typename A> T* insert(A* task)
    {
        static_assert( std::is_base_of<Action, A>::value, "T is not action." );
        _actions.push_back(task);
        return static_cast<T*>(this);
    }

    virtual ~ActionCompositor()
    {
        for( auto value : _actions ) delete value;
    }

    virtual void on_spawn(ActionExecutor& executor) override
    {
        for( auto value : _actions ) value->on_spawn(executor);
    }

    virtual void reset() override
    {
        for( auto value : _actions ) value->reset();
    }

protected:
    std::vector<Action*> _actions;
};

struct ActionSequence : public ActionCompositor<ActionSequence>
{
    virtual void reset() override;
    virtual void update(float) override;
    virtual bool is_finished() const override;

protected:
    size_t _current = 0;
};

struct ActionParallel : public ActionCompositor<ActionParallel>
{
    virtual void reset() override;
    virtual void update(float) override;
    virtual bool is_finished() const override;

protected:
    bool _finished = false;
};

/// misc
struct ActionWaitForSeconds : public ActionFiniteTime
{
    STATIC_SPAWN(ActionWaitForSeconds);
    ActionWaitForSeconds(float);
};

struct ActionClosure : Action
{
    using closure = std::function<bool()>;

    STATIC_SPAWN(ActionClosure);
    ActionClosure(const closure&);

    virtual void reset() override;
    virtual void update(float) override;
    virtual bool is_finished() const override;

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