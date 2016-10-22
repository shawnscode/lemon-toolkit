// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <core/core.hpp>

NS_LEMON_CORE_BEGIN

namespace details
{
    std::unique_ptr<SubsystemContext> s_context;
    Status s_status = Status::IDLE;

    bool initialize()
    {
        auto context = std::unique_ptr<SubsystemContext>(new (std::nothrow) SubsystemContext());
        if( context.get() == nullptr || !context->initialize() )
            return false;

        s_context = std::move(context);
        s_status = Status::RUNNING;
        return true;
    }

    Status status()
    {
        return s_status;
    }

    void dispose()
    {
        if( s_context.get() )
        {
            s_context->dispose();
            s_context.reset();
        }
        s_status = Status::DISPOSED;
    }

    SubsystemContext& context()
    {
        return *s_context;
    }
}

bool is_main_thread()
{
    return false;
}

NS_LEMON_CORE_END