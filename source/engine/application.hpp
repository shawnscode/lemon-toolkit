// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/context.hpp>

NS_FLOW2D_BEGIN

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

    // spawn a new subsystem with type S and construct arguments
    template<typename S, typename ... Args> void add_subsystem(Args&& ...);
    // retrieve the registered system instance, existence should be guaranteed
    template<typename S> S& get_subsystem();
    template<typename S> const S& get_subsystem() const;
    // check if we have specified subsystems
    template<typename ... Args> bool has_subsystems() const;
    // emit event with default dispatcher
    template<typename E> void emit(const E&);
    template<typename E, typename ... Args> void emit(Args && ... args);
    // subscribe/unsubscribe this to receive events of type E
    template<typename T, typename E> void subscribe();
    template<typename T, typename E> void unsubscribe();

    // return default entity manager
    core::EntityManager& get_world();
    const core::EntityManager& get_world() const;
    // return default event dispatcher
    core::EventManager&  get_dispatcher();
    const core::EventManager&  get_dispatcher() const;

protected:
    core::Context   _context;
    int             _exitcode;
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

/// IMPLEMENTATIONS of INLINE APPLICATION
INLINE core::EntityManager& Application::get_world()
{
    return _context.get_world();
}

INLINE core::EventManager&  Application::get_dispatcher()
{
    return _context.get_dispatcher();
}

INLINE const core::EntityManager& Application::get_world() const
{
    return _context.get_world();
}

INLINE const core::EventManager&  Application::get_dispatcher() const
{
    return _context.get_dispatcher();
}

template<typename S, typename ... Args>
INLINE void Application::add_subsystem(Args && ... args)
{
    _context.add_subsystem<S>(std::forward<Args>(args)...);
}

template<typename ... Args>
INLINE bool Application::has_subsystems() const
{
    return _context.has_subsystems<Args...>();
}

template<typename S>
INLINE S& Application::get_subsystem()
{
    return _context.get_subsystem<S>();
}

template<typename S>
INLINE const S& Application::get_subsystem() const
{
    return _context.get_subsystem<S>();
}

template<typename E>
INLINE void Application::emit(const E& evt)
{
    get_dispatcher().emit<E>(evt);
}

template<typename E, typename ... Args>
INLINE void Application::emit(Args && ... args)
{
    get_dispatcher().emit<E>(std::forward<Args>(args)...);
}

template<typename T, typename E>
INLINE void Application::subscribe()
{
    ENSURE( dynamic_cast<T*>(this) != nullptr );
    get_dispatcher().subscribe<E>(*static_cast<T*>(this));
}

template<typename T, typename E>
INLINE void Application::unsubscribe()
{
    ENSURE( dynamic_cast<T*>(this) != nullptr );
    get_dispatcher().unsubscribe<E>(*static_cast<T*>(this));
}

NS_FLOW2D_END