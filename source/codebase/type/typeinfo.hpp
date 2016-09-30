// @date 2016/09/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <type_traits>

NS_LEMON_BEGIN

struct TypeInfo
{
    using index_type = size_t;

    template<typename Base, typename Derived> static index_type id()
    {
        static_assert( std::is_base_of<Base, Derived>::value, "D should be derived from B." );
        static index_type sid = counter<Base>::value ++;
        return sid;
    }

protected:
    template<typename Base> struct counter
    {
        static index_type value;
    };
};

template<typename B> TypeInfo::index_type TypeInfo::counter<B>::value = 0;

struct TypeInfoGeneric
{
    using index_type = size_t;

    template<typename Base, typename Type> static index_type id()
    {
        static index_type sid = counter<Base>::value ++;
        return sid;
    }

protected:
    template<typename Base> struct counter
    {
        static index_type value;
    };
};

template<typename B> TypeInfo::index_type TypeInfoGeneric::counter<B>::value = 0;

NS_LEMON_END