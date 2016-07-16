#pragma once

#define NS_FLOW2D_MATH_BEGIN namespace flow2d { namespace math {
#define NS_FLOW2D_MATH_END } }


NS_FLOW2D_MATH_BEGIN

const static float nan = std::numeric_limits<float>::quiet_NaN();
const static float inf = std::numeric_limits<float>::infinity();

NS_FLOW2D_MATH_END