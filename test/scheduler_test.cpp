#include <catch.hpp>
#include <hayai.hpp>
#include <lemon-toolkit.hpp>

#include <thread>

using namespace lemon;

std::mutex  s_mutex;

static void fib(unsigned& out, unsigned index)
{
    // some heavy works might
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // 
    std::lock_guard<std::mutex> u(s_mutex);
    out += index;
}

struct Context
{
    Context()
    {
        core::task::initialize(4);
    }

    ~Context()
    {
        core::task::dispose();
    }
};

TEST_CASE_METHOD(Context, "TestSchedulerParalle")
{
    unsigned result = 0, cmp = 0;
    for( unsigned i = 1; i < 10; i++ )
    {
        cmp += i;
        auto handle = core::create_task("worker", fib, std::ref(result), i);
        core::run_task(handle);
        core::wait_task(handle);
        REQUIRE( cmp == result );
    }
    REQUIRE( cmp == result );

    auto master = core::create_task("master", fib, std::ref(result), 0);
    for( unsigned i = 1; i < 10; i++ )
    {
        cmp += i;
        auto handle = core::create_task_as_child(master, "slaver", fib, std::ref(result), i);
        core::run_task(handle);
    }
    core::run_task(master);
    core::wait_task(master);
    REQUIRE( cmp == result );
}

BENCHMARK(TaskScheduler, Sequence, 2, 5)
{
    unsigned result = 0;
    for( unsigned i = 1; i < 10; i++ )
        fib(result, i);
}

BENCHMARK(TaskScheduler, Paralle, 2, 5)
{
    Context context;

    unsigned result = 0;
    auto master = core::create_task("master", fib, std::ref(result), 0);
    for( unsigned i = 1; i < 10; i++ )
        core::run_task(core::create_task_as_child(master, "worker", fib, std::ref(result), i));
    core::run_task(master);
    core::wait_task(master);
}
