// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#include <core/private/scheduler.hpp>

#include <SDL2/SDL.h>

NS_LEMON_CORE_BEGIN

bool Scheduler::initialize(unsigned nworker)
{
    if( nworker == 0 )
        nworker = SDL_GetCPUCount() - 1;

    nworker = std::max(nworker, (uint32_t)1);
    _stop = false;
    for( uint32_t i = 0; i < nworker; i++ )
    {
        _workers.emplace_back(thread_run, std::ref(*this), i+1);
        _thread_indices.insert(std::make_pair(_workers.back().get_id(), i+1));
    }

    _thread_main = std::this_thread::get_id();
    _thread_indices.insert(std::make_pair(_thread_main, 0));
    return true;
}

void Scheduler::dispose()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
    }

    _condition.notify_all();
    for( auto& thread : _workers )
        thread.join();
}

TaskHandle Scheduler::create_task(const char* name, std::function<void()> closure)
{
    TaskHandle handle = create_task_chunk();
    Task* task = get_task(handle);

    task->closure = closure;
    task->jobs.store(1);
    strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));
    return handle;
}

TaskHandle Scheduler::create_task_as_child(TaskHandle parent, const char* name, std::function<void()> closure)
{
    TaskHandle handle = create_task_chunk();
    Task* task = get_task(handle);

    task->closure = closure;
    task->jobs.store(1);
    strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));

    Task* ptask = get_task(parent);
    if( ptask != nullptr )
    {
        uint32_t current_jobs = ptask->jobs++;
        if( current_jobs > 0 ) task->parent = parent;
        else ptask->jobs --;
    }
    return handle;
}

TaskHandle Scheduler::create_task_chunk()
{
    std::unique_lock<std::mutex> L(_allocator_mutex);

    if( _free_tasks.size() > 0 )
    {
        auto index = _free_tasks.back();
        _free_tasks.pop_back();
        return TaskHandle(index, _tasks[index].version);
    }

    _tasks.emplace_back();
    auto index = _tasks.size() - 1;
    return TaskHandle(index, _tasks[index].version);
}

Task* Scheduler::get_task(TaskHandle handle)
{
    auto& task = _tasks[handle.index];
    return task.version == handle.version ? &task : nullptr;
}

void Scheduler::run_task(TaskHandle handle)
{
    Task* task = get_task(handle);
    ASSERT( task != nullptr && task->jobs.load() > 0, "invalid task handle to run." );

    {
        std::unique_lock<std::mutex> L(_mutex);
        _alive_tasks.push(handle);
    }

    _condition.notify_one();
}

bool Scheduler::is_task_completed(TaskHandle handle)
{
    Task* task = get_task(handle);
    if( task == nullptr )
        return true;
    return task->jobs.load() == 0;
}

void Scheduler::wait_task(TaskHandle handle)
{
    Task* task = get_task(handle);
    if( task == nullptr )
        return;

    unsigned index = get_thread_index();
    while( !is_task_completed(handle) )
    {
        std::this_thread::yield();
        if( !execute_one_task(index, false) )
            break;
    }
}

void Scheduler::finish_task(TaskHandle handle)
{
    Task* task = get_task(handle);
    if( task == nullptr )
        return;

    // atomic decrement
    const uint32_t jobs = -- task->jobs;
    if( jobs == 0 )
    {
        finish_task(task->parent);

        // free captured reference and recycle task
        task->closure = nullptr;
        task->parent.version = 0; // invalidate parent handle
        task->version ++;

        {
            std::unique_lock<std::mutex> L(_allocator_mutex);
            _free_tasks.push_back(handle.index);
        }
    }
}

bool Scheduler::execute_one_task(unsigned index, bool wait)
{
    TaskHandle handle;
    {
        std::unique_lock<std::mutex> L(_mutex);

        if( wait )
        {
            _condition.wait(L, [=]
            {
                return _stop || !_alive_tasks.empty();
            });
        }

        if( _stop && _alive_tasks.empty() )
            return false;

        if( !wait && _alive_tasks.empty() )
            return true;

        handle = _alive_tasks.front();
        _alive_tasks.pop();

    }

    Task* task = get_task(handle);
    if( task == nullptr )
        ENSURE( task != nullptr );

    if( on_task_start )
        on_task_start(index, task->name);

    task->closure();
    finish_task(handle);

    if( on_task_stop )
        on_task_stop(index, task->name);

    return true;
}

unsigned Scheduler::get_thread_index() const
{
    auto found = _thread_indices.find(std::this_thread::get_id());
    if( found != _thread_indices.end() ) return found->second;
    return 0xFFFFFFFF;
}

void Scheduler::thread_run(Scheduler& scheduler, unsigned index)
{
    if( scheduler.on_thread_start )
        scheduler.on_thread_start(index);

    for( ;; )
    {
        if( !scheduler.execute_one_task(index, true) )
            break;
    }

    if( scheduler.on_thread_stop )
        scheduler.on_thread_stop(index);
}

NS_LEMON_CORE_END