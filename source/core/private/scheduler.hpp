// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/task.hpp>

#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>

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

    std::function<void()> closure = nullptr;
    std::atomic<uint32_t> jobs;
    TaskHandle parent;
    int16_t version = 1;
    char name[64] = {0};
};

// a light-weight task scheduler with automatic load balancing,
// the dependencies between tasks are addressed as parent-child relationships.
struct Scheduler
{
    // initialize task scheduler with specified worker count
    bool initialize(unsigned nworker);
    // shutdown task scheduler, this would block main thread until all the tasks finished
    void dispose();

    // create_task
    TaskHandle create_task(const char*, std::function<void()>);

    // create_task_as_child comes with parent-child relationships:
    // 1. a task should be able to have N child tasks;
    // 2. waiting for a task to be completed must properly synchronize across its children
    // as well
    TaskHandle create_task_as_child(TaskHandle, const char*, std::function<void()>);

    // run_task insert a task into a queue instead of executing it immediately
    void run_task(TaskHandle);

    // wait_task
    void wait_task(TaskHandle);

    // returns true if task completed
    bool is_task_completed(TaskHandle);

    // returns main thread id
    std::thread::id get_main_thread() const { return _thread_main; }

public:
    // several callbacks instended for thread initialization and profilers
    using thread_callback = std::function<void(unsigned)>;
    thread_callback on_thread_start;
    thread_callback on_thread_stop;
    // several callbacks instended for task based profiling
    using task_callback = std::function<void(unsigned, const char*)>;
    task_callback on_task_start;
    task_callback on_task_stop;

protected:
    static void thread_run(Scheduler&, unsigned index);
    Task* get_task(TaskHandle);
    TaskHandle create_task_chunk();
    void finish_task(TaskHandle);
    bool execute_one_task(unsigned, bool);
    unsigned get_thread_index() const;

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

NS_LEMON_CORE_END