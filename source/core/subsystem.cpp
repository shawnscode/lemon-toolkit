// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#include <core/subsystem.hpp>

NS_LEMON_CORE_BEGIN

bool SubsystemContext::initialize()
{
    return true;
}

void SubsystemContext::dispose()
{
    for( auto pair : _subsystems )
    {
        pair.second->dispose();
        delete pair.second;
    }

    _subsystems.clear();
}

NS_LEMON_CORE_END
