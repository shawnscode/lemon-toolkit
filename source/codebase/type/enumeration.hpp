// @date 2016/09/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

NS_LEMON_BEGIN

template<typename Enum>
struct EnableBitMaskOperators
{
    static const bool enable = false;
};

#define ENABLE_BITMASK_OPERATORS(E)  \
    template<> struct lemon::EnableBitMaskOperators<E> { static const bool enable = true; }; \

NS_LEMON_END

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

template<typename Enum> UReturns<Enum> value (Enum e)
{
    return static_cast<typename std::underlying_type<Enum>::type>(e);
}

template<typename Enum> using MaskReturns = typename std::enable_if<
    lemon::EnableBitMaskOperators<Enum>::enable,
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
