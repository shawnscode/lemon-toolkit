// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <core/instance.hpp>
#include <core/private/message.hpp>
#include <core/private/task.hpp>
#include <core/private/ecs.hpp>
#include <core/private/subsystem.hpp>
#include <SDL2/SDL.h>

NS_LEMON_CORE_BEGIN

std::unique_ptr<Scheduler> s_scheduler;
std::unique_ptr<SubsystemContext> s_context;
std::unique_ptr<Messenger> s_messenger;
std::unique_ptr<EntityComponentSystem> s_world;
Status s_status = Status::IDLE;

bool initialize(unsigned nworker)
{
    auto scheduler = std::unique_ptr<Scheduler>(new (std::nothrow) Scheduler());
    if( scheduler.get() == nullptr || !scheduler->initialize(nworker) )
        return false;

    auto dispatcher = std::unique_ptr<Messenger>(new (std::nothrow) Messenger());
    if( dispatcher.get() == nullptr || !dispatcher->initialize() )
        return false;

    auto context = std::unique_ptr<SubsystemContext>(new (std::nothrow) SubsystemContext());
    if( context.get() == nullptr || !context->initialize() )
        return false;

    auto world = std::unique_ptr<EntityComponentSystem>(new (std::nothrow) EntityComponentSystem());
    if( world.get() == nullptr || !world->initialize() )
        return false;

    s_scheduler = std::move(scheduler);
    s_messenger = std::move(dispatcher);
    s_context = std::move(context);
    s_world = std::move(world);
    s_status = Status::RUNNING;
    return true;
}

Status status()
{
    return s_status;
}

EntityComponentSystem& world()
{
    return *s_world;
}

SubsystemContext& context()
{
    return *s_context;
}

Messenger& messenger()
{
    return *s_messenger;
}

void dispose()
{
    s_status = Status::DISPOSED;

    if( s_world.get() )
    {
        s_world->dispose();
        s_world.reset();
    }

    if( s_context.get() )
    {
        s_context->dispose();
        s_context.reset();
    }

    if( s_messenger.get() )
    {
        s_messenger->dispose();
        s_messenger.reset();
    }

    if( s_scheduler.get() )
    {
        s_scheduler->dispose();
        s_scheduler.reset();
    }
}

// SCHEDULER

namespace internal
{
    TaskHandle create_task(const char* name, std::function<void()> task)
    {
        return s_scheduler->create_task(name, task);
    }

    TaskHandle create_task_as_child(TaskHandle parent, const char* name, std::function<void()> task)
    {
        return s_scheduler->create_task_as_child(parent, name, task);
    }
}

TaskHandle create_task(const char* name)
{
    return s_scheduler->create_task(name, nullptr);
}

void run_task(TaskHandle handle)
{
    s_scheduler->run_task(handle);
}

void wait_task(TaskHandle handle)
{
    s_scheduler->wait_task(handle);
}

bool is_completed(TaskHandle handle)
{
    return s_scheduler->is_task_completed(handle);
}

bool is_main_thread()
{
    return s_scheduler->get_main_thread() == std::this_thread::get_id();
}

uint32_t get_cpu_count()
{
    return SDL_GetCPUCount();
}

NS_LEMON_CORE_END