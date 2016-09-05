// @date 2016/09/01
// @author Mao Jingkai(oammix@gmail.com)

#include <core/event.hpp>

NS_LEMON_CORE_BEGIN

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
    static size_t index;
};

struct Dispatcher
{
    using closure = std::function<void(const void*)>;
    using dispatcher = std::unordered_map<size_t, closure>;
    std::vector<dispatcher> table;
};

static Dispatcher* s_dispatcher = nullptr;

namespace event
{
    bool initialize()
    {
        ASSERT( s_dispatcher == nullptr, "duplicated initialization of dispatcher." );

        s_dispatcher = new (std::nothrow) Dispatcher;
        if( s_dispatcher == nullptr )
            return false;

        return true;
    }

    void dispose()
    {
        delete s_dispatcher;
        s_dispatcher = nullptr;
    }

    void subscribe(uint32_t index, size_t id, closure cb)
    {
        if( s_dispatcher->table.size() <= index )
            s_dispatcher->table.resize(index+1);

        s_dispatcher->table[index].insert(std::make_pair(id, cb));
    }

    void unsubscribe(uint32_t index, size_t id)
    {
        if( s_dispatcher->table.size() > index )
        {
            auto found = s_dispatcher->table[index].find(id);
            if( found != s_dispatcher->table[index].end() )
                s_dispatcher->table[index].erase(found);
        }
    }

    void emit(uint32_t index, const void* evt)
    {
        if( s_dispatcher->table.size() > index )
        {
            for( auto& pair : s_dispatcher->table[index] )
                pair.second(evt);
        }
    }

    uint32_t s_event_index = 0;
}

NS_LEMON_CORE_END