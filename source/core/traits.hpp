// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>

NS_FLOW2D_BEGIN

struct TypeID
{
    using index_type = size_t;

    template<typename Base, typename Derived>
    static index_type value()
    {
        static_assert( std::is_base_of<Base, Derived>::value, "D should be derived from B." );
        static index_type id =  Counter<Base>::value ++;
        return id;
    }

protected:
    template<typename B> struct Counter
    {
        static index_type value;
    };
};

template<typename B> TypeID::index_type TypeID::Counter<B>::value = 0;

NS_FLOW2D_END