// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/subsystem.hpp>

NS_LEMON_CORE_BEGIN

// retrieve the registered system instance, existence should be guaranteed
template<typename S> S* get_subsystem();

// spawn a new subsystem with type S and construct arguments
template<typename S, typename ... Args> S* add_subsystem(Args&& ... args);

// release and unregistered a subsystem from our context
template<typename S> void remove_subsystem();

// check if we have specified subsystems
template<typename ... Args> bool has_subsystems();

//
bool is_main_thread();

namespace details
{
    enum class Status : uint8_t
    {
        IDLE,
        RUNNING,
        DISPOSED
    };

    bool initialize();
    Status status();
    void dispose();
    SubsystemContext& context();
}

template<typename S> S* get_subsystem()
{
    return details::context().get_subsystem<S>();
}

template<typename S, typename ... Args> S* add_subsystem(Args&& ... args)
{
    return details::context().add_subsystem<S>(std::forward<Args>(args)...);
}

template<typename S> void remove_subsystem()
{
    details::context().remove_subsystem<S>();
}

template<typename ... Args> bool has_subsystems()
{
    return details::context().has_subsystems<Args...>();
}

NS_LEMON_CORE_END