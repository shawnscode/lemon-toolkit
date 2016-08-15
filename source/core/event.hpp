// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/typeinfo.hpp>

NS_FLOW2D_CORE_BEGIN

// handles event subscription and delivery.
// subscriptions are automatically removed when receivers are destroyed.
struct EventManager
{
    // non-copyable
    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // subscribe an object to receive events of type E
    template<typename E, typename R> void subscribe(R&);
    template<typename E, typename R> void unsubscribe(R&);

    template<typename E> void emit(const E&);
    template<typename E, typename ... Args> void emit(Args && ... args);

protected:
    using closure       = std::function<void(const void*)>;
    using dispatcher    = std::unordered_map<size_t, closure>;
    std::vector<dispatcher> m_dispatchers;

    // not thread-safe
    template<typename T> static size_t get_event_index() { static size_t eid = index++; return eid; }
    static size_t index;
};

template<typename E, typename R>
INLINE void EventManager::subscribe(R& receiver)
{
    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = get_event_index<E>();
    auto id = (size_t)(&receiver);

    if( m_dispatchers.size() <= index )
        m_dispatchers.resize(index+1);

    m_dispatchers[get_event_index<E>()][id] = [&](const void* event)
    {
        receiver.receive(*static_cast<const E*>(event));
    };
}

template<typename E, typename R>
INLINE void EventManager::unsubscribe(R& receiver)
{
    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = get_event_index<E>();
    auto id = (size_t)(&receiver);

    if( m_dispatchers.size() > index )
    {
        auto found = m_dispatchers[index].find(id);
        if( found != m_dispatchers[index].end() )
            m_dispatchers[index].erase(found);
    }
}

template<typename E>
INLINE void EventManager::emit(const E& evt)
{
    auto index = get_event_index<E>();
    if( m_dispatchers.size() > index )
    {
        for( auto pair : m_dispatchers[index] )
            pair.second(static_cast<const void*>(&evt));
    }
}

template<typename E, typename ... Args>
INLINE void EventManager::emit(Args && ... args)
{
    auto index = get_event_index<E>();
    if( m_dispatchers.size() > index )
    {
        E evt = E(std::forward<Args>(args) ...);
        for( auto pair : m_dispatchers[index] )
            pair.second(static_cast<void*>(&evt));
    }
}

NS_FLOW2D_CORE_END