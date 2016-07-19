// @date 2016/07/19
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>

#define NS_FLOW2D_MATH_BEGIN namespace flow2d { namespace math {
#define NS_FLOW2D_MATH_END } }

NS_FLOW2D_MATH_BEGIN

const static float nan = std::numeric_limits<float>::quiet_NaN();
const static float inf = std::numeric_limits<float>::infinity();

const static float pi = 3.1415926;

float degree_to_radians(float);
float radians_to_degree(float);

#include <math/math.inl>

NS_FLOW2D_MATH_END