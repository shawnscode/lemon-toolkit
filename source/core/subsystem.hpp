// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/ecs.hpp>
#include <core/event.hpp>
#include <core/private/subsystem.hpp>

NS_LEMON_CORE_BEGIN

// returns subsystem context instance
SubsystemContext& context();

// retrieve the registered system instance, existence should be guaranteed
template<typename S> S* get_subsystem()
{
    return context().get_subsystem<S>();
}

// spawn a new subsystem with type S and construct arguments
template<typename S, typename ... Args> S* add_subsystem(Args&& ... args)
{
    return context().add_subsystem<S>(std::forward<Args>(args)...);
}

// release and unregistered a subsystem from our context
template<typename S> void remove_subsystem()
{
    context().remove_subsystem<S>();
}

// check if we have specified subsystems
template<typename ... Args> bool has_subsystems()
{
    return context().has_subsystems<Args...>();
}

template<typename ... Args> struct SubsystemWithEntities : public Subsystem
{
    using tuple = std::tuple<Args*...>;
    using collection = std::unordered_map<Handle, tuple>;
    using visitor = std::function<void(Handle, Args&...)>;
    using iterator = typename collection::iterator;
    using const_iterator = typename collection::const_iterator;

    virtual bool initialize() override;
    virtual void dispose() override;

    void receive(const EvtEntityModified&);
    void visit(const visitor&);

    iterator begin() { return _entities.begin(); }
    iterator end() { return _entities.end(); }
    const_iterator begin() const { return _entities.begin(); }
    const_iterator end() const { return _entities.end(); }

protected:
    collection _entities;
};

//
template<typename ... Args>
bool SubsystemWithEntities<Args...>::initialize()
{
   for( auto object : find_entities_with<Args...>() )
       _entities[object] = get_components<Args...>(object);
   subscribe<EvtEntityModified>(*this);
   return true;
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::dispose()
{
   unsubscribe<EvtEntityModified>(*this);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::receive(const EvtEntityModified& evt)
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

NS_LEMON_CORE_END