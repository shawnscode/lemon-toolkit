// @date 2016/10/18
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <atomic>

NS_LEMON_BEGIN

struct SpinMutex
{
    SpinMutex() = default;
    SpinMutex(const SpinMutex&) = delete;
    SpinMutex& operator = (const SpinMutex&) = delete;

    void lock()
    {
        while(_lock.test_and_set(std::memory_order_acquire)) {}
    }

    void unlock()
    {
        _lock.clear(std::memory_order_release);
    }

protected:
    std::atomic_flag _lock = ATOMIC_FLAG_INIT;
};

NS_LEMON_END
