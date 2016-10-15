// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/type/type_traits.hpp>

#include <unordered_map>

NS_LEMON_CORE_BEGIN

struct SubsystemContext;
struct Subsystem
{
    Subsystem() = default;
    Subsystem(const Subsystem&) = delete;
    Subsystem& operator = (const Subsystem&) = delete;
    virtual ~Subsystem() {}

    virtual bool initialize() { return true; }
    virtual void dispose() {}
};

struct SubsystemContext
{
    bool initialize();
    void dispose();

    // spawn a new subsystem with type S and construct arguments
    template<typename S, typename ... Args> S* add_subsystem(Args&& ...);

    // retrieve the registered system instance, existence should be guaranteed
    template<typename S> S* get_subsystem();

    // release and unregistered a subsystem from our context
    template<typename S> void remove_subsystem();

    // check if we have specified subsystems
    template<typename S> bool has_subsystems() const;
    template<typename S1, typename S2, typename ... Args> bool has_subsystems() const;

protected:
    std::unordered_map<TypeInfo::index_t, Subsystem*> _subsystems;
};

//
// IMPLEMENTATIONS of SUBSYSTEMS
template<typename S, typename ... Args> S* SubsystemContext::add_subsystem(Args&& ... args)
{
    auto index = TypeInfo::id<Subsystem, S>();
    ASSERT( !has_subsystems<S>(),
        "duplicated subsystem: %s.", typeid(S).name() );

    auto sys = new (std::nothrow) S(std::forward<Args>(args)...);
    ASSERT( sys->initialize(),
        "failed to initialize subsystem: %s.", typeid(S).name() );

    _subsystems.insert(std::make_pair(index, sys));
    return sys;
}

template<typename S> S* SubsystemContext::get_subsystem()
{
    auto index = TypeInfo::id<Subsystem, S>();

    auto found = _subsystems.find(index);
    if( found != _subsystems.end() )
        return static_cast<S*>(found->second);

    return nullptr;
}

template<typename S> void SubsystemContext::remove_subsystem()
{
    auto index = TypeInfo::id<Subsystem, S>();

    auto found = _subsystems.find(index);
    if( found != _subsystems.end() )
    {
        found->second->dispose();
        delete found->second;
        _subsystems.erase(found);
    }   
}

template<typename S> bool SubsystemContext::has_subsystems() const
{
    auto index = TypeInfo::id<Subsystem, S>();
    return _subsystems.find(index) != _subsystems.end();
}

template<typename S1, typename S2, typename ... Args> bool SubsystemContext::has_subsystems() const
{
    return has_subsystems<S1>() && has_subsystems<S2, Args...>();
}

NS_LEMON_CORE_END
