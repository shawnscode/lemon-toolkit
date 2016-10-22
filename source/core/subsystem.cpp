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
    for( auto iter = _orders.rbegin(); iter != _orders.rend(); iter ++ )
    {
        auto found = _subsystems.find(*iter);
        ENSURE(found != _subsystems.end());

        found->second->dispose();
        delete found->second;
        _subsystems.erase(found);
    }

    _orders.clear();
    ENSURE(_subsystems.size() == 0);
}

NS_LEMON_CORE_END
