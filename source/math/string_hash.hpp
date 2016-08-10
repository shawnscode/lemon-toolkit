// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/defines.hpp>

NS_FLOW2D_MATH_BEGIN

// 32-bit hash value for a string
struct StringHash
{
    static uint32_t calculate(const char* str);

    StringHash() : _value(0) {}
    StringHash(const char* str) : _value(calculate(str)) {}
    StringHash(const std::string& str) : _value(calculate(str.c_str())) {}

    StringHash(const StringHash&) = default;
    StringHash& operator = (const StringHash&) = default;

    // test for equality with another hash
    bool operator == (const StringHash& rhs) const { return _value == rhs._value; }
    bool operator == (const char* rhs) const { return _value == calculate(rhs); }
    bool operator == (const std::string& rhs) const { return _value == calculate(rhs.c_str()); }
    operator uint32_t () const { return _value; }

    uint32_t get_hash() const { return _value; }

protected:
    uint32_t _value;
};

NS_FLOW2D_MATH_END

namespace std
{
    template<> struct hash<flow2d::math::StringHash>
    {
        std::size_t operator() (const flow2d::math::StringHash& hash) const
        {
            return static_cast<std::size_t>(hash.get_hash());
        }
    };
}