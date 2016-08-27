#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <hayai.hpp>

// https://github.com/philsquared/Catch/blob/master/docs/slow-compiles.md

int main(int argc, char* const argv[])
{
    // run unit-test
    int utest = Catch::Session().run(argc, argv);
    if( utest != 0 ) return utest;

    // run benchmark-test
    hayai::ConsoleOutputter consoleOutputter;
    hayai::Benchmarker::AddOutputter(consoleOutputter);
    hayai::Benchmarker::RunAllTests();
    return 0;
}