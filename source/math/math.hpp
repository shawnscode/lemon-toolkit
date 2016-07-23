// @date 2016/07/19
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>

#define NS_FLOW2D_MATH_BEGIN namespace flow2d { namespace math {
#define NS_FLOW2D_MATH_END } }

NS_FLOW2D_MATH_BEGIN

const static float pi       = 3.1415926535f;
const static float pi_div_2 = pi / 2.f;

template<typename T> T max() { return std::numeric_limits<T>::max(); }
template<typename T> T min() { return std::numeric_limits<T>::min(); }
template<typename T> T nan() { return std::numeric_limits<T>::quiet_NaN(); }
template<typename T> T inf() { return std::numeric_limits<T>::infinity(); }
template<typename T> T epslion() { return std::numeric_limits<T>::epsilon(); }

float degree_to_radians(float);
float radians_to_degree(float);

#include <math/math.inl>

NS_FLOW2D_MATH_END