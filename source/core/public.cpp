// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <core/public.hpp>
#include <core/private/scheduler.hpp>
#include <core/private/context.hpp>
#include <core/private/dispatcher.hpp>
#include <core/private/world.hpp>
#include <SDL2/SDL.h>

NS_LEMON_CORE_BEGIN

std::auto_ptr<Scheduler> s_scheduler;
std::auto_ptr<Context> s_context;
std::auto_ptr<Dispatcher> s_dispatcher;
std::auto_ptr<World> s_world;

bool initialize(unsigned nworker)
{
    auto scheduler = std::auto_ptr<Scheduler>(new (std::nothrow) Scheduler());
    if( scheduler.get() == nullptr || !scheduler->initialize(nworker) )
        return false;

    auto dispatcher = std::auto_ptr<Dispatcher>(new (std::nothrow) Dispatcher());
    if( dispatcher.get() == nullptr || !dispatcher->initialize() )
        return false;

    auto context = std::auto_ptr<Context>(new (std::nothrow) Context());
    if( context.get() == nullptr || !context->initialize() )
        return false;

    auto world = std::auto_ptr<World>(new (std::nothrow) World());
    if( world.get() == nullptr || !world->initialize() )
        return false;

    s_scheduler = std::move(scheduler);
    s_dispatcher = std::move(dispatcher);
    s_context = std::move(context);
    s_world = std::move(world);
    return true;
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

// WORLD

namespace internal
{
    bool has_component_registered(TypeInfo::index_t id)
    {
        return s_world->has_component_registered(id);
    }

    bool register_component(TypeInfo::index_t id, size_t size, size_t chunk_size, const destructor& cb)
    {
        return s_world->register_component(id, size, chunk_size, cb);
    }

    void* add_component(TypeInfo::index_t id, Entity object)
    {
        return s_world->add_component(id, object);
    }

    void* get_component(TypeInfo::index_t id, Entity object)
    {
        return s_world->get_component(id, object);
    }

    void remove_component(TypeInfo::index_t id, Entity object)
    {
        s_world->remove_component(id, object);
    }

    bool has_component(TypeInfo::index_t id, Entity object)
    {
        return s_world->has_component(id, object);
    }

    Entity::index_t find_next_available(Entity::index_t id, ComponentMask mask, bool self)
    {
        return s_world->find_next_available(id, mask, self).get_index();
    }

    Entity get(Entity::index_t id)
    {
        return s_world->get(id);
    }
    
    namespace test_mem
    {
        size_t size(TypeInfo::index_t id)
        {
            return s_world->size(id);
        }
        
        size_t capacity(TypeInfo::index_t id)
        {
            return s_world->capacity(id);
        }
    }
}

Entity spawn()
{
    return s_world->spawn();
}

void recycle(Entity object)
{
    s_world->recycle(object);
}

bool alive(Entity object)
{
    return s_world->alive(object);
}

size_t size_of_world()
{
    return s_world->size();
}

void reset_world()
{
    s_world->dispose();
}

void* add_component(TypeInfo::index_t id, Entity object)
{
    return s_world->add_component(id, object);
}

void* get_component(TypeInfo::index_t id, Entity object)
{
    return s_world->get_component(id, object);
}

void remove_component(TypeInfo::index_t id, Entity object)
{
    s_world->remove_component(id, object);
}

bool has_component(TypeInfo::index_t id, Entity object)
{
    return s_world->has_component(id, object);
}

ComponentMask get_components_mask(Entity object)
{
    return s_world->get_components_mask(object);
}

Entity::index_t find_next_available(Entity::index_t index, ComponentMask mask, bool self)
{
    return s_world->find_next_available(index, mask, self).get_index();
}

Entity get(Entity::index_t index)
{
    return s_world->get(index);
}

NS_LEMON_CORE_END