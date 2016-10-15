#include <catch.hpp>
#include <hayai.hpp>
#include <lemon-toolkit.hpp>

#include <thread>

using namespace lemon;

std::mutex s_mutex;

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
    core::JobSystem task;
};

TEST_CASE_METHOD(Context, "TestSchedulerParalle")
{
   unsigned result = 0, cmp = 0;
   for( unsigned i = 1; i < 10; i++ )
   {
       cmp += i;
       auto handle = task.create("worker", fib, std::ref(result), i);
       task.run(handle);
       task.wait(handle);
       REQUIRE( cmp == result );
   }
   REQUIRE( cmp == result );

   auto master = task.create("master");
   for( unsigned i = 1; i < 10; i++ )
   {
       cmp += i;
       auto handle = task.create_as_child(master, "slaver", fib, std::ref(result), i);
       task.run(handle);
   }
   task.run(master);
   task.wait(master);
   REQUIRE( cmp == result );
}

BENCHMARK(TaskTest, Sequence, 2, 1)
{
   unsigned result = 0;
   for( unsigned i = 1; i < 10; i++ )
       fib(result, i);
}

BENCHMARK(TaskTest, JobSystemParralle, 2, 1)
{
   Context context;

   unsigned result = 0;
   auto master = context.task.create("master");
   char name[10];
   for( unsigned i = 1; i < 10; i++ )
   {
       sprintf(name, "worker_%d", i);
       context.task.run(context.task.create_as_child(master, name, fib, std::ref(result), i));
   }
   context.task.run(master);
   context.task.wait(master);
}
