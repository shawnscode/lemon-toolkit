#include <math/math.hpp>

NS_FLOW2D_MATH_BEGIN

// BKDR hash function
uint32_t StringHash::calculate(const char* str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

NS_FLOW2D_MATH_END