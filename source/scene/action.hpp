// @date 2016/07/20
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/components.hpp>
#include <core/system.hpp>
#include <scene/transform.hpp>
#include <math/vector.hpp>

struct ActionFixture;

NS_FLOW2D_BEGIN

struct ActionExecutor : ComponentWithEnvironment<>
{
    ~ActionExecutor();

    void run(Action*, bool override = false);
    void stop();
    void update(float);
    bool is_finished() const;

    ActionSystem& get_environment();

protected:
    friend class ActionSystem;

    ActionSystem*   _environment = nullptr;
    Action*         _task   = nullptr;
    bool            _paused = false;
};

struct Action
{
    virtual ~Action() {}
    virtual void start(ActionExecutor&) {}
    virtual void update(float) = 0;
    virtual bool is_finished() const = 0;

    void* operator new(size_t, MemoryChunks*);
    void operator delete(void*);

protected:
    friend class ActionSystem;

    // action memory is always managed by ActionSystem
    void* operator new(size_t);
    void* operator new[](size_t);
    void operator delete[](void*);

    Action() {};
    Action(const Action&) = delete;
    Action& operator = (const Action&) = delete;

private:
    MemoryChunks* _recycle = nullptr;
};

struct ActionTransform : public Action
{
    ActionTransform(float t, TransformSpace space) : _times(t), _space(space) {}

    virtual void start(ActionExecutor&) override;

    bool is_finished() const override
    {
        return _eplased >= _times;
    }

    void update(float dt) override
    {
        if( _eplased < _times )
        {
            _eplased += dt;
            step(std::min(_eplased/_times, 1.0f));
        }
    }

    virtual void step(float) = 0;

protected:
    Transform*      _transform = nullptr;
    TransformSpace  _space = TransformSpace::SELF;
    float           _times = 0.f, _eplased = 0.f;
};

struct ActionMoveTo : public ActionTransform
{
    ActionMoveTo(float t, const Vector2f& p, TransformSpace space = TransformSpace::SELF)
    : ActionTransform(t, space), _to(p) {}

    void start(ActionExecutor& executor) override
    {
        ActionTransform::start(executor);
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
    ActionMoveBy(float t, const Vector2f& p, TransformSpace space = TransformSpace::SELF)
    : ActionTransform(t, space), _to(p) {}

    void start(ActionExecutor& executor) override
    {
        ActionTransform::start(executor);
        _from = _transform->get_position(_space);
    }

    void step(float t) override
    {
        _transform->set_position(_from + t*_to, _space);
    }

protected:
    Vector2f _from, _to;
};

struct ActionSystem : SystemWithEntities<ActionExecutor>
{
    ActionSystem(EntityManager& world) : SystemWithEntities(world) {}

    void receive(const EvtComponentAdded<ActionExecutor>&);
    void receive(const EvtComponentRemoved<ActionExecutor>&);

    void update(float) override;

    template<typename T> size_t get_size_of()
    {
        auto found = _memories.find(sizeof(T));
        if( found == _memories.end() )
            return 0;
        return _memories[sizeof(T)]->size();
    }

    template<typename T, typename ... Args> T* spawn(Args && ... args)
    {
        auto found = _memories.find(sizeof(T));
        if( found == _memories.end() )
            _memories.insert( std::make_pair(sizeof(T), new MemoryChunks(sizeof(T) + sizeof(MemoryChunks*), 64)) );

        return new (_memories[sizeof(T)]) T(std::forward<Args>(args)...);
    }

protected:
    std::unordered_map<size_t, MemoryChunks*> _memories;
};

NS_FLOW2D_END