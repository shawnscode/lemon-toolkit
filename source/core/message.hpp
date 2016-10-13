// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/private/message.hpp>

NS_LEMON_CORE_BEGIN

Messenger& messenger();

// subscribe/unsubscribe an object to receive events of type E
template<typename E, typename R> void subscribe(R&);
template<typename E, typename R> void subscribe(R*);
template<typename E, typename R> void unsubscribe(R&);
template<typename E, typename R> void unsubscribe(R*);

// emit a constructed event to all subscribtions
template<typename E> void emit(const E&);
template<typename E, typename ... Args> void emit(Args && ... args);

//
// implementation of messenger
template<typename E, typename R> void subscribe(R& receiver)
{
    messenger().subscribe<E>(receiver);
}

template<typename E, typename R> void subscribe(R* receiver)
{
    messenger().subscribe<E>(*receiver);
}

template<typename E, typename R> void unsubscribe(R& receiver)
{
    messenger().unsubscribe<E>(receiver);
}

template<typename E, typename R> void unsubscribe(R* receiver)
{
    messenger().unsubscribe<E>(*receiver);
}

template<typename E> void emit(const E& event)
{
    messenger().emit<E>(event);
}

template<typename E, typename ... Args> void emit(Args && ... args)
{
    messenger().emit<E>(E(std::forward<Args>(args)...));
}

NS_LEMON_CORE_END