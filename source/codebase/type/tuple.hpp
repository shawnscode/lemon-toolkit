// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

NS_LEMON_BEGIN

/// to support tuple unpacking
template <typename T, T... ints> struct integer_sequence { };

template <typename T, T N, typename = void>
struct MakeIntegerSequenceImpl
{
    template <typename> struct Resolver;

    template <T... Prev> struct Resolver<integer_sequence<T, Prev...>>
    {
        using type = integer_sequence<T, Prev..., N-1>;
    };

    using type = typename Resolver<typename MakeIntegerSequenceImpl<T, N-1>::type>::type;
};

template <typename T, T N>
struct MakeIntegerSequenceImpl<T, N, typename std::enable_if<N==0>::type>
{
    using type = integer_sequence<T>;
};

template <typename T, T N>
using make_integer_sequence = typename MakeIntegerSequenceImpl<T, N>::type;

NS_LEMON_END
