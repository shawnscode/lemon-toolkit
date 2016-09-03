// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/typeinfo.hpp>

NS_LEMON_CORE_BEGIN

// subscribe an object to receive events of type E
template<typename E, typename R> void subscribe(R&);
template<typename E, typename R> void unsubscribe(R&);

// emit a constructed to all subscribtions
template<typename E> void emit(const E&);
template<typename E, typename ... Args> void emit(Args && ... args);


namespace event
{
    bool initialize();
    void dispose();

    extern uint32_t s_event_index;
    template<typename T> uint32_t get_event_index()
    {
        static uint32_t index = s_event_index++;
        return index;
    }

    using closure = std::function<void(const void*)>;
    void subscribe(uint32_t, size_t, closure);
    void unsubscribe(uint32_t, size_t);
    void emit(uint32_t, const void*);
}

template<typename E, typename R>
INLINE void subscribe(R& receiver)
{
    ASSERT_MAIN_THREAD("subscribe");

    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = event::get_event_index<E>();
    auto id = (size_t)(&receiver);

    event::subscribe(index, id, [&](const void* event)
    {
        receiver.receive(*static_cast<const E*>(event));
    });
}

template<typename E, typename R>
INLINE void unsubscribe(R& receiver)
{
    ASSERT_MAIN_THREAD("unsubscribe");

    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = event::get_event_index<E>();
    auto id = (size_t)(&receiver);

    event::unsubscribe(index, id);
}

template<typename E> INLINE void emit(const E& evt)
{
    ASSERT_MAIN_THREAD("emit");

    auto index = event::get_event_index<E>();
    event::emit(index, static_cast<const void*>(&evt));
}

template<typename E, typename ... Args> INLINE void emit(Args && ... args)
{
    ASSERT_MAIN_THREAD("emit");

    auto index = event::get_event_index<E>();
    E evt = E(std::forward<Args>(args)...);
    event::emit(index, static_cast<const void*>(&evt));
}

NS_LEMON_CORE_END