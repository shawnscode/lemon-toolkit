// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/subsystem.hpp>

NS_LEMON_BEGIN

struct Application
{
    Application();

    // setup before engine initialization. this is a chance to eg. modify the engine parameters.
    virtual void setup() {}
    // setup after engine initialization and before running the main loop
    virtual void start() {}
    // cleanup after the main loop
    virtual void stop() {}

    // initialize the engine and run the main loop, then return the
    // application exit code
    int run();
    // show  an error message, terminate the main loop, and set failure exit code
    void terminate_with_error(const std::string&);

protected:
    int _exitcode;
};

// define a platform-specific main function, which in turn executes the user-defined function
// android or iOS: use SDL_main
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
#define DEFINE_MAIN(function) \
extern "C" int SDL_main(int argc, char** argv); \
int SDL_main(int argc, char** argv) \
{ \
    return function; \
}
// linux or OS X: use main
#else
#define DEFINE_MAIN(function) \
int main(int argc, char** argv) \
{ \
    return function; \
}
#endif

// macro for defining a main function which creates a Context and the application, then runs it
#ifndef PLATFORM_IOS
#define DEFINE_APPLICATION_MAIN(T) \
int run_application() \
{ \
    std::unique_ptr<T> application(new T()); \
    return application->run(); \
} \
DEFINE_MAIN(run_application());
#else
// On iOS we will let this function exit, so do not hold the context and application in SharedPtr's
#define DEFINE_APPLICATION_MAIN(T) \
int run_application() \
{ \
    application = new T(); \
    return application->run(); \
} \
DEFINE_MAIN(run_application());
#endif

NS_LEMON_END