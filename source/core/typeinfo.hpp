// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>

NS_FLOW2D_CORE_BEGIN

///
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

/// to support tuple unpacking
template <typename T, T... ints>
struct integer_sequence { };

template <typename T, T N, typename = void>
struct make_integer_sequence_impl
{
    template <typename>
    struct tmp;

    template <T... Prev>
    struct tmp<integer_sequence<T, Prev...>>
    {
        using type = integer_sequence<T, Prev..., N-1>;
    };

    using type = typename tmp<typename make_integer_sequence_impl<T, N-1>::type>::type;
};

template <typename T, T N>
struct make_integer_sequence_impl<T, N, typename std::enable_if<N==0>::type>
{ using type = integer_sequence<T>; };

template <typename T, T N>
using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;

NS_FLOW2D_CORE_END