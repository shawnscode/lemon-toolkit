// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/context.hpp>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

NS_LEMON_CORE_BEGIN

// a light-weight task scheduler with automatic load balancing,
// the dependencies between tasks are addressed as parent-child relationships.
struct TaskScheduler : core::Subsystem
{
    SUBSYSTEM("TaskScheduler");

    TaskScheduler(core::Context& c, unsigned thread) : Subsystem(c), _thread_count(thread) {}
    virtual ~TaskScheduler() {}

    // initialize the whole scheduler and backup threads
    bool initialize() override;
    void dispose() override;

    template<typename F, typename ... Args>
    std::future<typename std::result_of<F(Args ...)>::type> add_task(F&&, Args&&...);

    // return the number of working thread
    unsigned get_worker_count() const { return _thread_count; }

public:
    // several callbacks instended for thread initialization and profilers
    using thread_callback = std::function<void(unsigned)>;
    thread_callback on_thread_start;
    thread_callback on_thread_stop;
    thread_callback on_wait_start;
    thread_callback on_wait_stop;

protected:
    static void thread_run(unsigned, TaskScheduler&);

    unsigned                            _thread_count;
    std::vector<std::thread>            _threads;
    std::queue<std::function<void()>>   _tasks;
    std::mutex                          _task_mutex;
    std::condition_variable             _condition;
    bool                                _stop;
};

template<typename F, typename ... Args>
std::future<typename std::result_of<F(Args ...)>::type> TaskScheduler::add_task(F&& functor, Args&& ... args)
{
    using return_type = typename std::result_of<F(Args ...)>::type;
    using task_type = std::packaged_task< return_type() >;

    auto functor_with_env = std::bind(std::forward<F>(functor), std::forward<Args>(args)...);
    auto task = std::shared_ptr<task_type>(new task_type( functor_with_env ));

    auto result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(_task_mutex);
        if( _stop )
            throw std::runtime_error("task insertion happens when scheduler disposed.");

        _tasks.emplace([task]() { (*task)(); });
    }

    _condition.notify_one();
    return result;
}

NS_LEMON_CORE_END