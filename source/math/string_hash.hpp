// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <math/math.hpp>

NS_LEMON_MATH_BEGIN

// 32-bit hash value for a string
struct StringHash
{
    static uint32_t calculate(const char* str);

    StringHash() : _value(0) {}
    StringHash(const char* str) : _value(calculate(str)) {}
    StringHash(const std::string& str) : _value(calculate(str.c_str())) {}

    StringHash(const StringHash&) = default;
    StringHash& operator = (const StringHash&) = default;
    StringHash& operator = (std::nullptr_t) { _value = 0; return *this; }

    // test for equality with another hash
    bool operator == (const StringHash& rhs) const { return _value == rhs._value; }
    bool operator == (const char* rhs) const { return _value == calculate(rhs); }
    bool operator == (const std::string& rhs) const { return _value == calculate(rhs.c_str()); }
    operator uint32_t () const { return _value; }

    bool is_empty() const { return _value == 0; }
    uint32_t get_hash() const { return _value; }

protected:
    uint32_t _value;
};

std::ostream& operator << (std::ostream&, const StringHash&);

NS_LEMON_MATH_END

namespace std
{
    template<> struct hash<lemon::math::StringHash>
    {
        std::size_t operator() (const lemon::math::StringHash& hash) const
        {
            return static_cast<std::size_t>(hash.get_hash());
        }
    };
}