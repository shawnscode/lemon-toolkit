// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/subsystem.hpp>

NS_LEMON_CORE_BEGIN

struct Context
{
    bool initialize();
    void dispose();

    // retrieve the registered system instance, existence should be guaranteed
    Subsystem* get_subsystem(TypeInfo::index_t);

    // spawn a new subsystem with type S and construct arguments
    void add_subsystem(TypeInfo::index_t, Subsystem*);

    // release and unregistered a subsystem from our context
    void remove_subsystem(TypeInfo::index_t);

    // check if we have specified subsystems
    bool has_subsystem(TypeInfo::index_t);

protected:
    std::unordered_map<TypeInfo::index_t, Subsystem*> _subsystems;
};

NS_LEMON_CORE_END