// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/entity.hpp>
#include <core/typeinfo.hpp>

NS_FLOW2D_CORE_BEGIN

struct Subsystem
{
    Subsystem(Context& c) : _context(c) {}
    virtual ~Subsystem() {}

    virtual void on_spawn() {}
    virtual void on_dispose() {}
    virtual void update(float) {}

protected:
    Context& _context;
};

template<typename ... Args>
struct SubsystemWithEntities : public Subsystem
{
    using tuple = std::tuple<Args*...>;
    using collection = std::unordered_map<Entity, tuple>;
    using visitor = std::function<void(Entity, Args&...)>;
    using iterator = typename collection::iterator;
    using const_iterator = typename collection::const_iterator;

    SubsystemWithEntities(Context& c) : Subsystem(c) {}

    virtual void on_spawn() override;
    virtual void on_dispose() override;

    void receive(const EvtEntityModified& );
    void visit(const visitor&);

    iterator begin() { return _entities.begin(); }
    iterator end() { return _entities.end(); }
    const_iterator begin() const { return _entities.begin(); }
    const_iterator end() const { return _entities.end(); }

protected:
    collection _entities;
};

struct Context
{
    Context() : _world(_dispatcher) {}
    Context(const Context&) = delete;
    Context& operator = (const Context&) = delete;

    EntityManager& get_world();
    EventManager&  get_dispatcher();
    template<typename S> S& get_subsystem();

    const EntityManager& get_world() const;
    const EventManager&  get_dispatcher() const;
    template<typename S> const S& get_subsystem() const;

    template<typename S, typename ... Args> void add_subsystem(Args&& ...);
    template<typename S> void remove_subsystem();
    void update(float);

protected:
    EntityManager   _world;
    EventManager    _dispatcher;

    std::unordered_map<TypeInfo::index_type, Subsystem*> _systems;
};

///
template<typename ... Args>
void SubsystemWithEntities<Args...>::on_spawn()
{
    auto& world = _context.get_world();
    for( auto object : world.template find_entities_with<Args...>() )
        _entities[object] = world.template get_components<Args...>(object);

    _context.get_dispatcher().template subscribe<EvtEntityModified>(*this);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::on_dispose()
{
    _context.get_dispatcher().template unsubscribe<EvtEntityModified>(*this);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::receive(const EvtEntityModified& evt)
{
    auto& world = _context.get_world();
    auto mask = world.template get_components_mask<Args...>();
    if( (world.get_components_mask(evt.object) & mask) == mask )
        _entities[evt.object] = world.template get_components<Args...>(evt.object);
    else
        _entities.erase(evt.object);
}

template<typename Func, typename Tup, std::size_t... index>
void visit_with_unpack(Func&& cb, Entity object, Tup&& tuple, integer_sequence<size_t, index...>)
{
    return cb(object, *std::get<index>(std::forward<Tup>(tuple))...);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::visit(const visitor& cb)
{
    constexpr auto Size = std::tuple_size<typename std::decay<tuple>::type>::value;
    for( auto pair : *this )
        visit_with_unpack(cb, pair.first, pair.second, make_integer_sequence<size_t, Size>{});
}

///
INLINE void Context::update(float dt)
{
    for( auto pair : _systems )
        pair.second->update(dt);
}

INLINE EntityManager& Context::get_world()
{
    return _world;
}

INLINE const EntityManager& Context::get_world() const
{
    return _world;
}

INLINE EventManager& Context::get_dispatcher()
{
    return _dispatcher;
}

INLINE const EventManager& Context::get_dispatcher() const
{
    return _dispatcher;
}

template<typename S> S& Context::get_subsystem()
{
    auto found = _systems.find(TypeInfo::id<Subsystem, S>());
    if( found != _systems.end() )
        return *static_cast<S*>(found->second);

    FATAL("get undefined subsystem.");
    return *static_cast<S*>(nullptr); // make compiler happy
}

template<typename S> const S& Context::get_subsystem() const
{
    auto found = _systems.find(TypeInfo::id<Subsystem, S>());
    if( found != _systems.end() )
        return *static_cast<S*>(found->second);

    FATAL("get undefined subsystem.");
    return *static_cast<S*>(nullptr); // make compiler happy
}

template<typename S, typename ... Args> void Context::add_subsystem(Args&& ... args)
{
    auto id = TypeInfo::id<Subsystem, S>();
    auto found = _systems.find(id);
    ASSERT( found == _systems.end(), "duplicated subsystem.");

    auto sys = new (std::nothrow) S(*this, std::forward<Args>(args) ...);
    sys->on_spawn();
    _systems.insert(std::make_pair(id, sys));
}

template<typename S> void Context::remove_subsystem()
{
    auto id = TypeInfo::id<Subsystem, S>();
    auto found = _systems.find(id);
    if( found != _systems.end() )
    {
        found.second->on_dispose();
        delete found->second;
        _systems.erase(found);
    }
}

NS_FLOW2D_CORE_END