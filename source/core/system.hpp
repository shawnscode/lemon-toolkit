// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>

NS_FLOW2D_BEGIN

struct System
{
    typedef size_t Type;

    virtual ~System() {}
    virtual void on_attach() {}
    virtual void on_detach() {}
    virtual void update(float) {}

    EntityManager&  world();
    EventManager&   dispatcher();
    SystemManager&  systems();

protected:
    friend class SystemManager;
    static Type s_type_counter;

    EntityManager*  _world         = nullptr;
    EventManager*   _dispatcher    = nullptr;
    SystemManager*  _systems       = nullptr;
};

template<typename T> struct SystemTrait : public System
{
    virtual ~SystemTrait() {}
    static Type type();
};

// an system for declaring component dependencies
template<typename C, typename ... Args>
struct RequireComponents : public SystemTrait<RequireComponents<C, Args...>>
{
    void on_attach() override;
    void on_detach() override;
    void receive(const EvtComponentAdded<C>&);

protected:
    template<typename D> void add_component(Entity);
    template<typename D, typename D1, typename ... Tails> void add_component(Entity);
};

struct SystemManager
{
    // non-copyable
    SystemManager(EntityManager&, EventManager&);
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    ~SystemManager();

    EventManager&   dispatcher();
    EntityManager&  world();

    void            update(float);

    template<typename S, typename ... Args> S* ensure(Args&& ... args);
    template<typename S, typename ... Args> S* add(Args&& ... args);
    template<typename S> void remove();
    template<typename S> S* get();

protected:
    EntityManager&                              _world;
    EventManager&                               _dispatcher;
    std::unordered_map<System::Type, System*>   _systems;
};

#include <core/system.inl>

NS_FLOW2D_END