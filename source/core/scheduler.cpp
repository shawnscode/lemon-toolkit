// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#include <core/scheduler.hpp>

NS_LEMON_CORE_BEGIN

bool TaskScheduler::initialize()
{
    if( _thread_count <= 0 )
        return false;

    _stop = false;
    for( unsigned i = 0; i < _thread_count; i++ )
        _threads.emplace_back(thread_run, i, std::ref(*this));
    return true;
}

void TaskScheduler::dispose()
{
    {
        std::unique_lock<std::mutex> lock(_task_mutex);
        _stop = true;
    }

    _condition.notify_all();
    for(auto& thread : _threads)
        thread.join();
}

void TaskScheduler::thread_run(unsigned index, TaskScheduler& scheduler)
{
    if( scheduler.on_thread_start )
        scheduler.on_thread_start(index);

    for( ;; )
    {
        if( scheduler.on_wait_start )
            scheduler.on_wait_start(index);

        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(scheduler._task_mutex);
            scheduler._condition.wait(lock, [&scheduler]
            {
                return scheduler._stop || !scheduler._tasks.empty();
            });

            if( scheduler._stop && scheduler._tasks.empty() )
                break;

            task = std::move(scheduler._tasks.front());
            scheduler._tasks.pop();
        }

        if( scheduler.on_wait_stop )
            scheduler.on_wait_stop(index);

        task();
    }

    if( scheduler.on_thread_stop )
        scheduler.on_thread_stop(index);
}

NS_LEMON_CORE_END