// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include "flow2d.hpp"
#include <vector>
#include <unordered_map>

NS_FLOW2D_BEGIN

// signal to slot pattern based on [simplesignal](https://testbit.eu/cpp11-signal-system-performance/)
struct EventDispatcher
{
    using Closure = std::function<void (const void*)>;

    struct ReceiverLink
    {
        ReceiverLink    *next = nullptr;
        ReceiverLink    *prev = nullptr;
        Closure         closure;
        explicit ReceiverLink(const Closure& c) : closure(c) {}
    };

    EventDispatcher() = default;
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator = (const EventDispatcher&) = delete;
    ~EventDispatcher();

    size_t  size() const;
    size_t  connect(const Closure&);
    bool    disconnect(size_t);
    void    emit(const void*);

protected:
    void    unlink(ReceiverLink*);
    size_t  add_before(ReceiverLink*, const Closure&);

    ReceiverLink *m_ring = nullptr;
};

struct Event
{
    typedef size_t Type;

protected:
    static Type s_type_counter;
};

template<typename T> struct EventTrait : public Event
{
    static Type type()
    {
        static Type t = s_type_counter ++;
        return t;
    }
};

// handles event subscription and delivery.
// subscriptions are automatically removed when receivers are destroyed.
struct EventManager
{
    typedef std::unordered_map<Event::Type, size_t> SubscripedEvents;
    typedef std::unordered_map<size_t, SubscripedEvents> ReceiverRecords;

    // non-copyable
    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
    ~EventManager();

    // subscribe an object to receive events of type E
    template<typename E, typename R> void subscribe(R&);
    template<typename E, typename R> bool unsubscribe(R&);

    template<typename E> void emit(const E&);
    template<typename E, typename ... Args> void emit(Args && ... args);

protected:
    EventDispatcher* get_dispatcher(Event::Type);

    std::vector<EventDispatcher*> m_dispatchers;
    ReceiverRecords               m_records;
};

#include "event.inl"
NS_FLOW2D_END