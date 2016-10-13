// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <limits>
#include <functional>

NS_LEMON_BEGIN

#define DECLARE_HANDLE(Name) \
    struct Name : public Handle { \
        Name() = default; \
        Name(index_t i, index_t v) : Handle(i, v) {} };

struct Handle
{
    using index_t = uint16_t;
    const static index_t invalid = std::numeric_limits<index_t>::max();

    Handle() = default;
    Handle(const Handle&) = default;
    Handle(index_t index, index_t version) : _index(index), _version(version) {}

    Handle& operator = (const Handle&) = default;

    index_t get_index() const { return _index; }
    index_t get_version() const { return _version; }

    // return true if this handle is valid
    bool is_valid() const
    {
        return _index != invalid && _version != invalid;
    }

    // returns true if index and version both equals
    bool operator == (const Handle& rhs) const
    {
        return (!is_valid() && !rhs.is_valid()) || (_index == rhs._index && _version == rhs._version);
    }

    bool operator != (const Handle& rhs) const
    {
        return !(*this == rhs);
    }

    // comparisons for sorted containers
    bool operator < (const Handle& rh) const
    {
        return _version == rh._version ? _index < rh._index : _version < rh._version;
    }

    // invalidate handle
    void invalidate()
    {
        _index = invalid;
        _version = invalid;
    }

protected:
    index_t _index = invalid;
    index_t _version = invalid;
};

NS_LEMON_END

namespace std
{
    template<> struct hash<lemon::Handle>
    {
        std::size_t operator() (const lemon::Handle& handle) const
        {
            return (((size_t)handle.get_version()) << 16) & ((size_t)handle.get_index());
        }
    };
}