// @date 2016/08/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>

NS_LEMON_CORE_BEGIN

// Task handle index
struct TaskHandle
{
    TaskHandle() : index(0), version(0) {}
    TaskHandle(uint16_t index, uint16_t version) : index(index), version(version) {}
    int16_t index;
    int16_t version;
};

// create_task
template<typename F, typename ... Args> TaskHandle create_task(const char*, F&&, Args&& ...);

// create_task_as_child comes with parent-child relationships:
// 1. a task should be able to have N child tasks;
// 2. waiting for a task to be completed must properly synchronize across its children
// as well
template<typename F, typename ... Args> TaskHandle create_task_as_child(TaskHandle, const char*, F&&, Args&&...);

// run_task insert a task into a queue instead of executing it immediately
void run_task(TaskHandle);

// wait_task
void wait_task(TaskHandle);

// returns true if task completed
bool is_task_completed(TaskHandle);

// returns true if we are under the thread execute task::initialize()
bool is_main_thread();

// returns the number of cpu core, it could be used as hint to initialize task scheduler
uint32_t get_cpu_count();

//
// implementation of traits
namespace internal
{
    TaskHandle create_task(const char*, std::function<void()>);
    TaskHandle create_task_as_child(TaskHandle, const char*, std::function<void()>);
}

template<typename F, typename ... Args>
TaskHandle create_task(const char* name, F&& functor, Args&& ... args)
{
    auto functor_with_env = std::bind(std::forward<F>(functor), std::forward<Args>(args)...);
    return internal::create_task(name, functor_with_env);
}

template<typename F, typename ... Args>
TaskHandle create_task_as_child(TaskHandle parent, const char* name, F&& functor, Args&&... args)
{
    auto functor_with_env = std::bind(std::forward<F>(functor), std::forward<Args>(args)...);
    return internal::create_task_as_child(parent, name, functor_with_env);
}

NS_LEMON_CORE_END