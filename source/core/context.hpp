// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/entity.hpp>
#include <core/typeinfo.hpp>

NS_FLOW2D_CORE_BEGIN

//
#define SUBSYSTEM(NAME) constexpr static char const* const name = NAME;

struct Subsystem
{
    SUBSYSTEM("NONAME");

    // initialize/dispose will be called when u spawn/release it with a context
    virtual bool initialize() { return true; }
    virtual void dispose() {}
    virtual void update(float) {}

    // retrieve the registered system instance, existence should be guaranteed
    template<typename S> S& get_subsystem();
    template<typename S> const S& get_subsystem() const;
    // check if we have specified subsystems
    template<typename ... Args> bool has_subsystems() const;

    EntityManager& get_world();
    EventManager&  get_dispatcher();
    const EntityManager& get_world() const;
    const EventManager&  get_dispatcher() const;

protected:
    friend class Context;

    Subsystem(Context& c) : _context(c) {}
    virtual ~Subsystem() {}

    Subsystem(const Subsystem&) = delete;
    Subsystem& operator = (const Subsystem&) = delete;

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

    SUBSYSTEM("SubsystemWithEntities");
    SubsystemWithEntities(Context& c) : Subsystem(c) {}

    virtual bool initialize() override;
    virtual void dispose() override;

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

    virtual ~Context()
    {
        for( auto pair : _subsystems )
        {
            pair.second->dispose();
            delete pair.second;
        }
        _subsystems.clear();
    }

    EntityManager& get_world();
    EventManager&  get_dispatcher();

    const EntityManager& get_world() const;
    const EventManager&  get_dispatcher() const;

    // retrieve the registered system instance, existence should be guaranteed
    template<typename S> S& get_subsystem();
    template<typename S> const S& get_subsystem() const;
    // spawn a new subsystem with type S and construct arguments
    template<typename S, typename ... Args> void add_subsystem(Args&& ...);
    // release and unregistered a subsystem from our context
    template<typename S> void remove_subsystem();
    // check if we have specified subsystems
    template<typename S> bool has_subsystems() const;
    template<typename S1, typename S2, typename ... Args> bool has_subsystems() const;

    // update all of the subsystems
    void update(float);

protected:
    using collection = std::vector<Subsystem*>;
    void resolve_nocycle();

    EntityManager   _world;
    EventManager    _dispatcher;
    std::unordered_map<TypeInfo::index_type, Subsystem*> _subsystems;
};

///

INLINE EntityManager& Subsystem::get_world()
{
    return _context.get_world();
}

INLINE EventManager&  Subsystem::get_dispatcher()
{
    return _context.get_dispatcher();
}

INLINE const EntityManager& Subsystem::get_world() const
{
    return _context.get_world();
}

INLINE const EventManager&  Subsystem::get_dispatcher() const
{
    return _context.get_dispatcher();
}

template<typename ... Args>
INLINE bool Subsystem::has_subsystems() const
{
    return _context.has_subsystems<Args...>();
}

template<typename S>
INLINE S& Subsystem::get_subsystem()
{
    return _context.get_subsystem<S>();
}

template<typename S>
INLINE const S& Subsystem::get_subsystem() const
{
    return _context.get_subsystem<S>();
}

template<typename ... Args>
bool SubsystemWithEntities<Args...>::initialize()
{
    auto& world = _context.get_world();
    for( auto object : world.template find_entities_with<Args...>() )
        _entities[object] = world.template get_components<Args...>(object);

    _context.get_dispatcher().template subscribe<EvtEntityModified>(*this);
    return true;
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::dispose()
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
    for( auto pair : _subsystems )
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
    auto found = _subsystems.find(TypeInfo::id<Subsystem, S>());
    if( found != _subsystems.end() )
        return *static_cast<S*>(found->second);

    FATAL( "trying to get unregistered subsystem: %s.", S::name );
    return *static_cast<S*>(nullptr); // make compiler happy
}

template<typename S> const S& Context::get_subsystem() const
{
    auto found = _subsystems.find(TypeInfo::id<Subsystem, S>());
    if( found != _subsystems.end() )
        return *static_cast<S*>(found->second);

    FATAL( "trying to get unregistered subsystem: %s.", S::name );
    return *static_cast<S*>(nullptr); // make compiler happy
}

template<typename S, typename ... Args> void Context::add_subsystem(Args&& ... args)
{
    auto id = TypeInfo::id<Subsystem, S>();
    auto found = _subsystems.find(id);
    ASSERT( found == _subsystems.end(), "duplicated subsystem: %s.", S::name );

    auto sys = new (std::nothrow) S(*this, std::forward<Args>(args) ...);
    ASSERT( sys->initialize(), "failed to initialize subsystem: %s.", S::name );
    _subsystems.insert(std::make_pair(id, sys));
}

template<typename S> void Context::remove_subsystem()
{
    auto id = TypeInfo::id<Subsystem, S>();
    auto found = _subsystems.find(id);
    if( found != _subsystems.end() )
    {
        found.second->dispose();
        delete found->second;
        _subsystems.erase(found);
    }
}

template<typename S> bool Context::has_subsystems() const
{
    auto id = TypeInfo::id<Subsystem, S>();
    auto found = _subsystems.find(id);
    return found != _subsystems.end();
}

template<typename S1, typename S2, typename ... Args>
bool Context::has_subsystems() const
{
    return has_subsystems<S1>() | has_subsystems<S2, Args...>();
}

NS_FLOW2D_CORE_END