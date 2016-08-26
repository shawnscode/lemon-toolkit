// @date 2016/07/19
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_FLOW2D_MATH_BEGIN namespace lemon { namespace math {
#define NS_FLOW2D_MATH_END } }

NS_FLOW2D_MATH_BEGIN

const static float pi       = 3.1415926535f;
const static float pi_div_2 = pi / 2.f;

template<typename T> T max() { return std::numeric_limits<T>::max(); }
template<typename T> T min() { return std::numeric_limits<T>::min(); }
template<typename T> T nan() { return std::numeric_limits<T>::quiet_NaN(); }
template<typename T> T inf() { return std::numeric_limits<T>::infinity(); }
template<typename T> T epsilon() { return std::numeric_limits<T>::epsilon(); }
template<typename T> T clamp(T s, T min, T max) { return std::max(std::min(s, max), min); }

template<typename T>
T to_radians(T degree) { return std::fmod(degree, (T)360) / (T)180 * pi; }
template<typename T>
T to_degree(T radians) { return std::fmod(radians, (T)pi*2) * (T)180 / pi; }

NS_FLOW2D_MATH_END