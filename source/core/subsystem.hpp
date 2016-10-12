// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/entity.hpp>
#include <codebase/type/tuple.hpp>

NS_LEMON_CORE_BEGIN

#define SUBSYSTEM(NAME) constexpr static char const* const name = NAME;

struct Subsystem
{
    SUBSYSTEM("lemon::core::Subsystem");
    Subsystem() {}
    virtual ~Subsystem() {}

    // initialize/dispose will be called when u spawn/release it with a context
    virtual bool initialize() { return true; }
    virtual void dispose() {}

    Subsystem(const Subsystem&) = delete;
    Subsystem& operator = (const Subsystem&) = delete;
};

template<typename ... Args> struct SubsystemWithEntities : public Subsystem
{
    using tuple = std::tuple<Args*...>;
    using collection = std::unordered_map<Handle, tuple>;
    using visitor = std::function<void(Handle, Args&...)>;
    using iterator = typename collection::iterator;
    using const_iterator = typename collection::const_iterator;

    SUBSYSTEM("lemon::core::SubsystemWithEntities");

    virtual bool initialize() override;
    virtual void dispose() override;

    void receive(const EvtHandleModified&);
    void visit(const visitor&);

    iterator begin() { return _entities.begin(); }
    iterator end() { return _entities.end(); }
    const_iterator begin() const { return _entities.begin(); }
    const_iterator end() const { return _entities.end(); }

protected:
    collection _entities;
};

// retrieve the registered system instance, existence should be guaranteed
template<typename S> S* get_subsystem();

// spawn a new subsystem with type S and construct arguments
template<typename S, typename ... Args> S* add_subsystem(Args&& ...);

// release and unregistered a subsystem from our context
template<typename S> void remove_subsystem();

// check if we have specified subsystems
template<typename S> bool has_subsystems();
template<typename S1, typename S2, typename ... Args> bool has_subsystems();

//
template<typename ... Args>
bool SubsystemWithEntities<Args...>::initialize()
{
    for( auto object : find_entities_with<Args...>() )
        _entities[object] = get_components<Args...>(object);
    subscribe<EvtHandleModified>(*this);
    return true;
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::dispose()
{
    unsubscribe<EvtHandleModified>(*this);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::receive(const EvtHandleModified& evt)
{
    // auto& world = _context.get_world();
    auto mask = get_components_mask<Args...>();
    if( (get_components_mask(evt.object) & mask) == mask )
        _entities[evt.object] = get_components<Args...>(evt.object);
    else
        _entities.erase(evt.object);
}

template<typename Func, typename Tup, std::size_t... index>
void visit_with_unpack(Func&& cb, Handle object, Tup&& tuple, integer_sequence<size_t, index...>)
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

//
// implementation of traits
namespace internal
{
    void add_subsystem(TypeInfo::index_t, Subsystem*);
    void remove_subsystem(TypeInfo::index_t);
    bool has_subsystem(TypeInfo::index_t);
    Subsystem* get_subsystem(TypeInfo::index_t);
}

template<typename S>
S* get_subsystem()
{
    auto index = TypeInfo::id<Subsystem, S>();
    return static_cast<S*>(internal::get_subsystem(index));
}

template<typename S, typename ... Args>
S* add_subsystem(Args&& ... args)
{
    auto index = TypeInfo::id<Subsystem, S>();
    ASSERT( !internal::has_subsystem(index), "duplicated subsystem: %s.", S::name );

    auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
    ASSERT( sys->initialize(), "failed to initialize subsystem: %s.", S::name );

    internal::add_subsystem(index, sys);
    return sys;
}

template<typename S>
void remove_subsystem()
{
    auto index = TypeInfo::id<Subsystem, S>();
    internal::remove_subsystem(index);
}

template<typename S>
bool has_subsystems()
{
    auto index = TypeInfo::id<Subsystem, S>();
    return internal::has_subsystem(index);
}

template<typename S1, typename S2, typename ... Args>
bool has_subsystems()
{
    return has_subsystems<S1>() | has_subsystems<S2, Args...>();
}

NS_LEMON_CORE_END