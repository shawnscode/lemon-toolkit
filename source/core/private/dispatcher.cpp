// @date 2016/09/01
// @author Mao Jingkai(oammix@gmail.com)

#include <core/private/dispatcher.hpp>

NS_LEMON_CORE_BEGIN

bool Dispatcher::initialize()
{
    return true;
}

void Dispatcher::dispose()
{
}

void Dispatcher::subscribe(TypeInfoGeneric::index_t index, size_t id, closure cb)
{
    if( _table.size() <= index )
        _table.resize(index+1);

    _table[index].insert(std::make_pair(id, cb));
}

void Dispatcher::unsubscribe(TypeInfoGeneric::index_t index, size_t id)
{
    if( _table.size() > index )
    {
        auto found = _table[index].find(id);
        if( found != _table[index].end() )
            _table[index].erase(found);
    }
}

void Dispatcher::emit(TypeInfoGeneric::index_t index, const void* evt)
{
    if( _table.size() > index )
    {
        for( auto& pair : _table[index] )
            pair.second(evt);
    }
}

NS_LEMON_CORE_END