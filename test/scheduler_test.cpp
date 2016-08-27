#include <catch.hpp>
#include <hayai.hpp>
#include <lemon-toolkit.hpp>

using namespace lemon;

std::mutex  s_mutex;

static unsigned fib(unsigned& out, unsigned index)
{
    // some heavy works might
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // 
    std::lock_guard<std::mutex> u(s_mutex);
    out += index;
    return out;
}

TEST_CASE_METHOD(core::Context, "TestSchedulerParalle")
{
    add_subsystem<core::TaskScheduler>(4);
    auto& scheduler = get_subsystem<core::TaskScheduler>();

    unsigned result = 0, cmp = 0;
    for( unsigned i = 1; i < 10; i++ )
    {
        cmp += i;
        REQUIRE( scheduler.add_task(fib, std::ref(result), i).get() == cmp );
    }
    REQUIRE( cmp == result );

    std::vector<std::future<unsigned>> futures;
    for( unsigned i = 1; i < 10; i++ )
    {
        cmp += i;
        futures.emplace_back( scheduler.add_task(fib, std::ref(result), i) );
    }
    for( auto& future : futures ) future.wait();
    REQUIRE( cmp == result );
}

BENCHMARK(TaskScheduler, Sequence, 2, 5)
{
    core::Context context;
    context.add_subsystem<core::TaskScheduler>(4);
    auto& scheduler = context.get_subsystem<core::TaskScheduler>();

    unsigned result = 0;
    for( unsigned i = 1; i < 10; i++ )
        scheduler.add_task(fib, std::ref(result), i).get();
}

BENCHMARK(TaskScheduler, Paralle, 2, 5)
{
    core::Context context;
    context.add_subsystem<core::TaskScheduler>(4);
    auto& scheduler = context.get_subsystem<core::TaskScheduler>();

    unsigned result = 0;
    std::vector<std::future<unsigned>> futures;
    for( unsigned i = 1; i < 10; i++ )
        futures.emplace_back( scheduler.add_task(fib, std::ref(result), i) );
    for( auto& future : futures ) future.wait();
}
