// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/entity.hpp>
#include <core/typeinfo.hpp>

NS_LEMON_CORE_BEGIN

#define SUBSYSTEM(NAME) constexpr static char const* const name = NAME;

struct Subsystem
{
    SUBSYSTEM("lemon::core::Subsystem");
    Subsystem() {}
    virtual ~Subsystem() {}

    // initialize/dispose will be called when u spawn/release it with a context
    virtual bool initialize() { return true; }
    virtual void dispose() {}

    Subsystem(const Subsystem&) = delete;
    Subsystem& operator = (const Subsystem&) = delete;
};

template<typename ... Args> struct SubsystemWithEntities : public Subsystem
{
    using tuple = std::tuple<Args*...>;
    using collection = std::unordered_map<Entity, tuple>;
    using visitor = std::function<void(Entity, Args&...)>;
    using iterator = typename collection::iterator;
    using const_iterator = typename collection::const_iterator;

    SUBSYSTEM("lemon::core::SubsystemWithEntities");

    virtual bool initialize() override;
    virtual void dispose() override;

    void receive(const EvtEntityModified&);
    void visit(const visitor&);

    iterator begin() { return _entities.begin(); }
    iterator end() { return _entities.end(); }
    const_iterator begin() const { return _entities.begin(); }
    const_iterator end() const { return _entities.end(); }

protected:
    collection _entities;
};

// retrieve the registered system instance, existence should be guaranteed
template<typename S> S& get_subsystem();

// spawn a new subsystem with type S and construct arguments
template<typename S, typename ... Args> S& add_subsystem(Args&& ...);

// release and unregistered a subsystem from our context
template<typename S> void remove_subsystem();

// check if we have specified subsystems
template<typename S> bool has_subsystems();
template<typename S1, typename S2, typename ... Args> bool has_subsystems();

namespace subsystem
{
    bool initialize();
    void dispose();
}

#include <core/subsystem.inl>
NS_LEMON_CORE_END