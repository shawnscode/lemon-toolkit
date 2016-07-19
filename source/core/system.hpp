// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <core/traits.hpp>

NS_FLOW2D_BEGIN

struct System
{
    System(EntityManager& world) : _world(world) {}
    virtual ~System() {}
    virtual void on_spawn(SystemManager&) {}
    virtual void on_dispose(SystemManager&) {}
    virtual void update(float) {}

protected:
    EntityManager& _world;
};

template<typename C>
struct SystemWithEntities : public System
{
    SystemWithEntities(EntityManager& world) : System(world) {}

    virtual void on_spawn(SystemManager&) override;
    virtual void on_dispose(SystemManager&) override;

    void receive(const EvtComponentAdded<C>&);
    void receive(const EvtComponentRemoved<C>&);

protected:
    std::unordered_map<Entity, C*> _entities;
};

struct SystemManager
{
    // non-copyable
    SystemManager(EntityManager&);
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    //
    ~SystemManager();
    EntityManager&  get_world();
    EventManager&   get_dispatcher();

    void            update(float);

    template<typename S, typename ... Args> S* ensure(Args&& ... args);
    template<typename S, typename ... Args> S* add(Args&& ... args);
    template<typename S> void remove();
    template<typename S> S* get();

protected:
    EntityManager& _world;
    std::unordered_map<TypeID::index_type, System*> _systems;
};

#include <core/system.inl>

NS_FLOW2D_END