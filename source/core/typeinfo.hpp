// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <type_traits>

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

template<typename Enum>
struct EnableBitMaskOperators
{
    static const bool enable = false;
};

#define ENABLE_BITMASK_OPERATORS(E)  \
    template<> struct lemon::core::EnableBitMaskOperators<E> { static const bool enable = true; }; \

NS_FLOW2D_CORE_END

struct EnumerationHash
{
    template <typename T> std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

template<typename Enum> using UReturns = typename std::enable_if<
    std::is_enum<Enum>::value,
    typename std::underlying_type<Enum>::type>::type;

template<typename Enum> UReturns<Enum> to_value (Enum e)
{
    return static_cast<typename std::underlying_type<Enum>::type>(e);
}

template<typename Enum> using MaskReturns = typename std::enable_if<
    lemon::core::EnableBitMaskOperators<Enum>::enable,
    Enum>::type;

template<typename Enum> MaskReturns<Enum> operator ~ (Enum lhs)
{
    using value_t = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (~static_cast<value_t>(lhs));
}

template<typename Enum> MaskReturns<Enum> operator & (Enum lhs, Enum rhs)
{
    using value_t = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (static_cast<value_t>(lhs) & static_cast<value_t>(rhs));
}

template<typename Enum> MaskReturns<Enum> operator &= (Enum lhs, Enum rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

template<typename Enum> MaskReturns<Enum> operator | (Enum lhs, Enum rhs)
{
    using value_t = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (static_cast<value_t>(lhs) | static_cast<value_t>(rhs));
}

template<typename Enum> MaskReturns<Enum> operator |= (Enum lhs, Enum rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<typename Enum> MaskReturns<Enum> operator ^ (Enum lhs, Enum rhs)
{
    using value_t = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (static_cast<value_t>(lhs) ^ static_cast<value_t>(rhs));
}

template<typename Enum> MaskReturns<Enum> operator ^= (Enum lhs, Enum rhs)
{
    lhs = lhs ^ rhs;
    return lhs;
}
