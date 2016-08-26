#include <math/string_hash.hpp>

NS_LEMON_MATH_BEGIN

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

std::ostream& operator << (std::ostream& out, const StringHash& hash)
{
    return out << "StringHash(" << hash.get_hash() << ")";
}

NS_LEMON_MATH_END