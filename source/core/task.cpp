// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#include <core/task.hpp>
#include <SDL2/SDL.h>

NS_LEMON_CORE_BEGIN

struct Task
{
    Task() {}
    Task(Task&& rhs) : jobs(rhs.jobs.load())
    {
        closure = std::move(rhs.closure);
        parent = rhs.parent;
        version = rhs.version;
        strncpy(name, rhs.name, strlen(rhs.name));
    }

    std::function<void()>   closure = nullptr;
    std::atomic<uint32_t>   jobs;
    TaskHandle              parent;
    int16_t                 version = 1;
    char                    name[64] = {0};
};

struct TaskScheduler
{
    std::mutex allocator_mutex;
    std::vector<Task> tasks;
    std::vector<int16_t> free_tasks;

    std::mutex mutex;
    std::queue<TaskHandle> alive_tasks;

    std::vector<std::thread> workers;
    std::condition_variable condition;
    bool stop;

    std::unordered_map<std::thread::id, unsigned> thread_indices;

    // callbacks
    task::thread_closure on_thread_start;
    task::thread_closure on_thread_stop;

    task::task_closure on_wait_start;
    task::task_closure on_wait_stop;
};

static TaskScheduler*   s_scheduler = nullptr;
static std::thread::id  s_main_thread;

static TaskHandle _create_task()
{
    std::unique_lock<std::mutex> L(s_scheduler->allocator_mutex);

    if( s_scheduler->free_tasks.size() > 0 )
    {
        auto index = s_scheduler->free_tasks.back();
        s_scheduler->free_tasks.pop_back();
        return TaskHandle(index, s_scheduler->tasks[index].version);
    }

    s_scheduler->tasks.emplace_back();
    auto index = s_scheduler->tasks.size() - 1;
    return TaskHandle(index, s_scheduler->tasks[index].version);
}

static Task* _get_task(TaskHandle handle)
{
    auto& task = s_scheduler->tasks[handle.index];
    return task.version == handle.version ? &task : nullptr;
}

static void _finish_task(TaskHandle handle)
{
    Task* task = _get_task(handle);
    if( task == nullptr )
        return;

    // atomic decrement
    const uint32_t jobs = -- task->jobs;
    if( jobs == 0 )
    {
        _finish_task(task->parent);

        // free captured reference and recycle task
        task->closure = nullptr;
        task->parent.version = 0; // invalidate parent handle
        task->version ++;

        {
            std::unique_lock<std::mutex> L(s_scheduler->allocator_mutex);
            s_scheduler->free_tasks.push_back(handle.index);
        }
    }
}

static bool _execute_one_task()
{
    TaskHandle handle;
    {
        std::unique_lock<std::mutex> L(s_scheduler->mutex);
        s_scheduler->condition.wait(L, [=]
        {
            return s_scheduler->stop || !s_scheduler->alive_tasks.empty();
        });

        if( s_scheduler->stop && s_scheduler->alive_tasks.empty() )
            return false;

        handle = s_scheduler->alive_tasks.front();
        s_scheduler->alive_tasks.pop();
    }

    Task* task = _get_task(handle);
    ENSURE( task != nullptr );

    auto found = s_scheduler->thread_indices.find(std::this_thread::get_id());
    if( s_scheduler->on_wait_stop )
        s_scheduler->on_wait_stop(found->second, task->name);

    task->closure();
    _finish_task(handle);

    if( s_scheduler->on_wait_start )
        s_scheduler->on_wait_start(found->second, task->name);
    return true;
}

bool completed_task(TaskHandle handle)
{
    Task* task = _get_task(handle);
    if( task == nullptr )
        return true;
    return task->jobs.load() == 0;
}

void run_task(TaskHandle handle)
{
    Task* task = _get_task(handle);
    ASSERT( task != nullptr && task->jobs.load() > 0, "invalid task handle to run." );

    {
        std::unique_lock<std::mutex> L(s_scheduler->mutex);
        s_scheduler->alive_tasks.push(handle);
    }
}

void wait_task(TaskHandle handle)
{
    Task* task = _get_task(handle);
    ASSERT( task != nullptr, "invalid task handle to wait." );

    while( !completed_task(handle) )
    {
        if( !_execute_one_task() )
            break;
    }
}

static void _thread_run()
{
    auto found = s_scheduler->thread_indices.find(std::this_thread::get_id());
    if( s_scheduler->on_thread_start )
        s_scheduler->on_thread_start(found->second);

    for( ;; )
    {
        if( !_execute_one_task() )
            break;
    }

    if( s_scheduler->on_thread_stop )
        s_scheduler->on_thread_stop(found->second);
}

namespace task
{
    TaskHandle create_task(const char* name, std::function<void()> closure)
    {
        TaskHandle handle = _create_task();
        Task* task = _get_task(handle);

        task->closure = closure;
        task->jobs.store(1);
        strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));
        return handle;
    }

    TaskHandle create_task_as_child(TaskHandle parent, const char* name, std::function<void()> closure)
    {
        TaskHandle handle = _create_task();
        Task* task = _get_task(handle);

        task->closure = closure;
        task->jobs.store(1);
        strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));

        Task* ptask = _get_task(parent);
        if( ptask != nullptr )
        {
            uint32_t current_jobs = ptask->jobs++;
            if( current_jobs > 0 ) task->parent = parent;
            else ptask->jobs --;
        }
        return handle;
    }

    uint32_t get_cpu_core_count()
    {
        return SDL_GetCPUCount();
    }

    bool is_main_thread()
    {
        return s_scheduler != nullptr && s_main_thread == std::this_thread::get_id();
    }

    bool initialize(uint32_t nworker)
    {
        if( nworker == 0 )
            nworker = get_cpu_core_count() - 1;

        nworker = std::max(nworker, (uint32_t)1);
        s_scheduler = new (std::nothrow) TaskScheduler;
        if( s_scheduler == nullptr )
            return false;

        s_scheduler->stop = false;
        for( uint32_t i = 0; i < nworker; i++ )
        {
            s_scheduler->workers.emplace_back(_thread_run);
            s_scheduler->thread_indices.insert(std::make_pair(s_scheduler->workers.back().get_id(), i+1));
        }

        s_main_thread = std::this_thread::get_id();
        s_scheduler->thread_indices.insert(std::make_pair(s_main_thread, 0));
        return true;
    }

    void dispose()
    {
        {
            std::unique_lock<std::mutex> lock(s_scheduler->mutex);
            s_scheduler->stop = true;
        }

        s_scheduler->condition.notify_all();
        for( auto& thread : s_scheduler->workers )
            thread.join();

        delete s_scheduler;
        s_scheduler = nullptr;
    }

    void set_on_thread_start(thread_closure& cb)
    {
        s_scheduler->on_thread_start = cb;
    }

    void set_on_thread_stop(thread_closure& cb)
    {
        s_scheduler->on_thread_stop = cb;
    }

    void set_on_wait_start(task_closure& cb)
    {
        s_scheduler->on_wait_start = cb;
    }

    void set_on_wait_stop(task_closure& cb)
    {
        s_scheduler->on_wait_stop = cb;
    }
}

NS_LEMON_CORE_END