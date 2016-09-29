// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#include <core/private/context.hpp>

NS_LEMON_CORE_BEGIN

bool Context::initialize()
{
    return true;
}

void Context::dispose()
{
    for( auto pair : _subsystems )
    {
        pair.second->dispose();
        delete pair.second;
    }
    _subsystems.clear();
}

Subsystem* Context::get_subsystem(TypeInfo::index_type index)
{
    auto found = _subsystems.find(index);
    if( found != _subsystems.end() )
        return found->second;

    return nullptr;
}

void Context::add_subsystem(TypeInfo::index_type index, Subsystem* subsystem)
{
    _subsystems.insert(std::make_pair(index, subsystem));
}

void Context::remove_subsystem(TypeInfo::index_type index)
{
    auto found = _subsystems.find(index);
    if( found != _subsystems.end() )
    {
        found->second->dispose();
        delete found->second;
        _subsystems.erase(found);
    }   
}

bool Context::has_subsystem(TypeInfo::index_type index)
{
    return _subsystems.find(index) != _subsystems.end();
}

NS_LEMON_CORE_END