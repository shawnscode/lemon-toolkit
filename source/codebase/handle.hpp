// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <limits>
#include <vector>

NS_LEMON_BEGIN

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
    bool is_valid() const { return _index != invalid && _version != invalid; }
    // returns true if index and version both equals
    bool operator == (const Handle& rhs) const { return _index == rhs._index && _version == rhs._version; }
    // invalidate handle
    void invalidate() { _index = invalid; _version = invalid; }

protected:
    index_t _index = invalid;
    index_t _version = invalid;
};

struct ReuseableHandleSet
{
    using index_t = Handle::index_t;

    Handle create()
    {
        if( _freeslots.size() != 0 )
        {
            index_t index = _freeslots.back();
            _freeslots.pop_back();
            return Handle(index, _versions[index]);
        }

        _versions.push_back(0);
        return Handle(_versions.size()-1, 0);
    }

    bool is_valid(Handle handle)
    {
        const index_t index = handle.get_index();
        const index_t version = handle.get_version();

        return index < _versions.size() && _versions[index] == version;
    }

    void free(Handle handle)
    {
        if( !is_valid(handle) )
            return;

        _versions[handle.get_index()]++;
        _freeslots.push_back(handle.get_index());
    }

protected:
    std::vector<index_t> _versions;
    std::vector<index_t> _freeslots;
};

NS_LEMON_END