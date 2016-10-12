// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <core/public.hpp>
#include <core/private/scheduler.hpp>
#include <core/private/context.hpp>
#include <core/private/dispatcher.hpp>
#include <SDL2/SDL.h>

NS_LEMON_CORE_BEGIN

std::unique_ptr<Scheduler> s_scheduler;
std::unique_ptr<Context> s_context;
std::unique_ptr<Dispatcher> s_dispatcher;
std::unique_ptr<EntityComponentSystem> s_world;

bool initialize(unsigned nworker)
{
    auto scheduler = std::unique_ptr<Scheduler>(new (std::nothrow) Scheduler());
    if( scheduler.get() == nullptr || !scheduler->initialize(nworker) )
        return false;

    auto dispatcher = std::unique_ptr<Dispatcher>(new (std::nothrow) Dispatcher());
    if( dispatcher.get() == nullptr || !dispatcher->initialize() )
        return false;

    auto context = std::unique_ptr<Context>(new (std::nothrow) Context());
    if( context.get() == nullptr || !context->initialize() )
        return false;

    auto world = std::unique_ptr<EntityComponentSystem>(new (std::nothrow) EntityComponentSystem());
    if( world.get() == nullptr || !world->initialize() )
        return false;

    s_scheduler = std::move(scheduler);
    s_dispatcher = std::move(dispatcher);
    s_context = std::move(context);
    s_world = std::move(world);
    return true;
}

bool is_running()
{
    return s_context != nullptr;
}

EntityComponentSystem& world()
{
    return *s_world;
}

void dispose()
{
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

    if( s_dispatcher.get() )
    {
        s_dispatcher->dispose();
        s_dispatcher.reset();
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

bool is_task_completed(TaskHandle handle)
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

// CONTEXT

namespace internal
{
    void add_subsystem(TypeInfo::index_t index, Subsystem* subsystem)
    {
        s_context->add_subsystem(index, subsystem);
    }

    void remove_subsystem(TypeInfo::index_t index)
    {
        s_context->remove_subsystem(index);
    }

    bool has_subsystem(TypeInfo::index_t index)
    {
        return s_context->has_subsystem(index);
    }

    Subsystem* get_subsystem(TypeInfo::index_t index)
    {
        return s_context->get_subsystem(index);
    }
}

// DISPATCHER

namespace internal
{
    void subscribe(TypeInfoGeneric::index_t index, size_t id, closure cb)
    {
        s_dispatcher->subscribe(index, id, cb);
    }

    void unsubscribe(TypeInfoGeneric::index_t index, size_t id)
    {
        s_dispatcher->unsubscribe(index, id);
    }

    void emit(TypeInfoGeneric::index_t index, const void* evt)
    {
        s_dispatcher->emit(index, evt);
    }
}

NS_LEMON_CORE_END