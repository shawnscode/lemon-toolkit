// @date 2016/09/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/type/type_traits.hpp>

#include <vector>
#include <functional>
#include <unordered_map>

NS_LEMON_CORE_BEGIN

struct Event {};

struct Messenger
{
    bool initialize() { return true; }
    void dispose() {}

    template<typename E, typename R> void subscribe(R& receiver)
    {
        const auto index = TypeInfoGeneric::id<Event, E>();
        if( _table.size() <= index )
            _table.resize(index+1);

        const auto id = (size_t)(&receiver);
        _table[index].insert(std::make_pair(id, [&](const void* event)
        {
            receiver.receive(*static_cast<const E*>(event));
        }));
    }

    template<typename E, typename R> void unsubscribe(R& receiver)
    {
        const auto index = TypeInfoGeneric::id<Event, E>();
        if( _table.size() <= index )
            return;

        const auto id = (size_t)(&receiver);
        auto found = _table[index].find(id);
        if( found != _table[index].end() )
            _table[index].erase(found);
    }

    template<typename E> void emit(const E& event)
    {
        auto index = TypeInfoGeneric::id<Event, E>();
        if( _table.size() <= index )
            return;

        for( auto& pair : _table[index] )
            pair.second(static_cast<const void*>(&event));
    }
    
protected:
    using closure = std::function<void(const void*)>;
    using mailbox = std::unordered_map<TypeInfoGeneric::index_t, closure>;
    std::vector<mailbox> _table;
};

NS_LEMON_CORE_END