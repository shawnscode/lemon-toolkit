// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/type/type_traits.hpp>

#include <functional>
#include <unordered_map>

NS_LEMON_CORE_BEGIN

// subscribe/unsubscribe an object to receive events of type E
template<typename E, typename R> void subscribe(R&);
template<typename E, typename R> void unsubscribe(R&);

// emit a constructed event to all subscribtions
template<typename E> void emit(const E&);
template<typename E, typename ... Args> void emit(Args && ... args);

//
// implementation of traits
namespace internal
{
    struct Event {};

    using closure = std::function<void(const void*)>;
    void subscribe(TypeInfoGeneric::index_t, size_t, closure);
    void unsubscribe(TypeInfoGeneric::index_t, size_t);
    void emit(TypeInfoGeneric::index_t, const void*);
}

template<typename E, typename R>
void subscribe(R& receiver)
{
    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = TypeInfoGeneric::id<internal::Event, E>();
    auto id = (size_t)(&receiver);

    internal::subscribe(index, id, [&](const void* event)
    {
        receiver.receive(*static_cast<const E*>(event));
    });
}

template<typename E, typename R>
void unsubscribe(R& receiver)
{
    ASSERT_MAIN_THREAD("unsubscribe");

    static_assert( sizeof(size_t) == sizeof(&receiver), "size of size_t is different with void*" );

    auto index = TypeInfoGeneric::id<internal::Event, E>();
    auto id = (size_t)(&receiver);

    internal::unsubscribe(index, id);
}

template<typename E>
void emit(const E& evt)
{
    auto index = TypeInfoGeneric::id<internal::Event, E>();
    internal::emit(index, static_cast<const void*>(&evt));
}

template<typename E, typename ... Args>
void emit(Args && ... args)
{
    auto index = TypeInfoGeneric::id<internal::Event, E>();
    E evt = E(std::forward<Args>(args)...);
    internal::emit(index, static_cast<const void*>(&evt));
}

NS_LEMON_CORE_END