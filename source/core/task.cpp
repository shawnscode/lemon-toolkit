// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#include <core/task.hpp>

#include <mutex>
#include <thread>
#include <condition_variable>

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
    bool initialize(unsigned nworker)
    {
        if( nworker == 0 )
            nworker = task::get_cpu_core_count() - 1;

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

    void dispose()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
        }

        _condition.notify_all();
        for( auto& thread : _workers )
            thread.join();
    }

    TaskHandle create_task_chunk()
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

    Task* get_task(TaskHandle handle)
    {
        auto& task = _tasks[handle.index];
        return task.version == handle.version ? &task : nullptr;
    }

    void run_task(TaskHandle handle)
    {
        Task* task = get_task(handle);
        ASSERT( task != nullptr && task->jobs.load() > 0, "invalid task handle to run." );

        {
            std::unique_lock<std::mutex> L(_mutex);
            _alive_tasks.push(handle);
        }

        _condition.notify_one();
    }

    void finish_task(TaskHandle handle)
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

    bool execute_one_task(unsigned index, bool wait)
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

        if( on_wait_stop )
            on_wait_stop(index, task->name);

        task->closure();
        finish_task(handle);

        if( on_wait_start )
            on_wait_start(index, task->name);

        return true;
    }

    unsigned get_thread_index() const
    {
        auto found = _thread_indices.find(std::this_thread::get_id());
        if( found != _thread_indices.end() ) return found->second;
        return 0xFFFFFFFF;
    }

    static void thread_run(TaskScheduler& scheduler, unsigned index)
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

    task::thread_closure on_thread_start;
    task::thread_closure on_thread_stop;

    task::task_closure on_wait_start;
    task::task_closure on_wait_stop;

protected:
    std::mutex _allocator_mutex;
    std::vector<Task> _tasks;
    std::vector<int16_t> _free_tasks;

    std::mutex _mutex;
    std::queue<TaskHandle> _alive_tasks;

    std::vector<std::thread> _workers;
    std::thread::id _thread_main;
    std::condition_variable _condition;
    bool _stop;

    std::unordered_map<std::thread::id, unsigned> _thread_indices;
};

static TaskScheduler*   s_scheduler = nullptr;

namespace task
{
    TaskHandle create_task(const char* name, std::function<void()> closure)
    {
        TaskHandle handle = s_scheduler->create_task_chunk();
        Task* task = s_scheduler->get_task(handle);

        task->closure = closure;
        task->jobs.store(1);
        strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));
        return handle;
    }

    TaskHandle create_task_as_child(TaskHandle parent, const char* name, std::function<void()> closure)
    {
        TaskHandle handle = s_scheduler->create_task_chunk();
        Task* task = s_scheduler->get_task(handle);

        task->closure = closure;
        task->jobs.store(1);
        strncpy(task->name, name, std::min(sizeof(task->name), strlen(name)));

        Task* ptask = s_scheduler->get_task(parent);
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
        return s_scheduler != nullptr && s_scheduler->get_thread_index() == 0;
    }

    bool initialize(uint32_t nworker)
    {
        ASSERT( s_scheduler == nullptr, "duplicated initialization of task scheduler." );

        TaskScheduler* scheduler = new (std::nothrow) TaskScheduler();
        if( scheduler != nullptr && scheduler->initialize(nworker) )
        {
            s_scheduler = scheduler;
            return true;
        }

        if( scheduler != nullptr ) delete scheduler;
        return false;
    }

    void dispose()
    {
        s_scheduler->dispose();

        delete s_scheduler;
        s_scheduler = nullptr;
    }

    void set_on_thread_start(const thread_closure& cb)
    {
        s_scheduler->on_thread_start = cb;
    }

    void set_on_thread_stop(const thread_closure& cb)
    {
        s_scheduler->on_thread_stop = cb;
    }

    void set_on_wait_start(const task_closure& cb)
    {
        s_scheduler->on_wait_start = cb;
    }

    void set_on_wait_stop(const task_closure& cb)
    {
        s_scheduler->on_wait_stop = cb;
    }
}

bool completed_task(TaskHandle handle)
{
    Task* task = s_scheduler->get_task(handle);
    if( task == nullptr )
        return true;
    return task->jobs.load() == 0;
}

void run_task(TaskHandle handle)
{
    s_scheduler->run_task(handle);
}

void wait_task(TaskHandle handle)
{
    Task* task = s_scheduler->get_task(handle);
    if( task == nullptr )
        return;

    unsigned index = s_scheduler->get_thread_index();
    while( !completed_task(handle) )
    {
        std::this_thread::yield();
        if( !s_scheduler->execute_one_task(index, false) )
            break;
    }
}

NS_LEMON_CORE_END