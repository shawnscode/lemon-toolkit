// @date 2016/09/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

NS_LEMON_BEGIN

struct TypeInfo
{
    using index_type = size_t;

    template<typename Base, typename Derived>
    static index_type id()
    {
        static_assert( std::is_base_of<Base, Derived>::value, "D should be derived from B." );
        static index_type sid = counter<Base>::value ++;
        return sid;
    }

protected:
    template<typename B> struct counter
    {
        static index_type value;
    };
};

template<typename B> TypeInfo::index_type TypeInfo::counter<B>::value = 0;

NS_LEMON_END