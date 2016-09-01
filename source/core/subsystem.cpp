// @date 2016/08/31
// @author Mao Jingkai(oammix@gmail.com)

#include <core/subsystem.hpp>
#include <core/event.hpp>

NS_LEMON_CORE_BEGIN

struct Context
{
    std::unordered_map<TypeInfo::index_type, Subsystem*> subsystems;
};

static Context* s_context = nullptr;

namespace subsystem
{
    bool initialize()
    {
        ASSERT( s_context == nullptr, "duplicated subsystem initialization." );
        s_context = new (std::nothrow) Context;
        return s_context != nullptr;
    }

    void shutdown()
    {
        delete s_context;
        s_context = nullptr;
    }
}

Subsystem* get_subsystem(TypeInfo::index_type index)
{
    auto found = s_context->subsystems.find(index);
    if( found != s_context->subsystems.end() )
        return found->second;

    return nullptr;
}

void add_subsystem(TypeInfo::index_type index, Subsystem* subsystem)
{
    s_context->subsystems.insert(std::make_pair(index, subsystem));
}

void remove_subsystem(TypeInfo::index_type index)
{
    auto found = s_context->subsystems.find(index);
    if( found != s_context->subsystems.end() )
    {
        found->second->dispose();
        delete found->second;
        s_context->subsystems.erase(found);
    }   
}

bool has_subsystem(TypeInfo::index_type index)
{
    return s_context->subsystems.find(index) != s_context->subsystems.end();
}

NS_LEMON_CORE_END