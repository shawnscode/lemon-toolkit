#include <catch.hpp>
#include <hayai.hpp>
#include <lemon-toolkit.hpp>

#include <thread>

using namespace lemon;

std::mutex  s_mutex;

unsigned fibonacci(unsigned n)
{
    if( n == 0 ) return 0;
    if( n == 1 ) return 1;

    return fibonacci(n-1) + fibonacci(n-2);
}

static void fib(unsigned& out, unsigned index)
{
    // some heavy works might
    fibonacci(30);

    // 
    {
        std::lock_guard<std::mutex> u(s_mutex);
        out += index;
    }
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

BENCHMARK(TaskScheduler, Sequence, 2, 1)
{
    unsigned result = 0;
    for( unsigned i = 1; i < 100; i++ )
        fib(result, i);
}

BENCHMARK(TaskScheduler, Paralle, 2, 1)
{
    Context context;

    // core::task::set_on_wait_stop([&](unsigned index, const char* task)
    // {
    //     printf("[+] task: %s at %d\n", task, index);
    // });

    // core::task::set_on_wait_start([=](unsigned index, const char* task)
    // {
    //     printf("[-] task: %s at %d\n", task, index);
    // });

    unsigned result = 0;
    auto master = core::create_task("master", fib, std::ref(result), 0);
    char name[10];
    for( unsigned i = 1; i < 10; i++ )
    {
        sprintf(name, "worker_%d", i);
        core::run_task(core::create_task_as_child(master, name, fib, std::ref(result), i));
    }
    core::run_task(master);
    core::wait_task(master);
}
