// @date 2016/09/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/type/typeinfo.hpp>

#include <vector>
#include <functional>
#include <unordered_map>

NS_LEMON_CORE_BEGIN

struct Dispatcher
{
    using closure = std::function<void(const void*)>;
    using dispatcher = std::unordered_map<size_t, closure>;

    bool initialize();
    void dispose();

    // subscribe an object to receive events
    void subscribe(TypeInfoGeneric::index_t, size_t, closure);
    // unsubscribe an object from dispatcher
    void unsubscribe(TypeInfoGeneric::index_t, size_t);
    // emit a contructed event to all subscribtions
    void emit(TypeInfoGeneric::index_t, const void*);

protected:
    std::vector<dispatcher> _table;
};

NS_LEMON_CORE_END