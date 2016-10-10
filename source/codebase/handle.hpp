// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <limits>

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

// struct ReuseableHandleSet
// {
//     using index_t 
// };

template<size_t M> struct HandleSet
{
    using index_t = Handle::index_t;

    HandleSet()
    {
        reset();
    }

    Handle allocate()
    {
        if( _num_handles < M )
        {
            index_t index = _num_handles ++;
            _sparse[_dense[index]] = index;
            return Handle(_dense[index], _versions[index]);
        }

        return Handle();
    }

    void free(Handle handle)
    {
        if( !is_valid(handle) )
            return;

        index_t index = _sparse[handle.get_index()];
        index_t temp = _dense[--_num_handles];

        _dense[_num_handles] = handle.get_index();
        _sparse[temp] = index;
        _dense[index] = temp;
        _versions[index] ++;
    }

    bool is_valid(Handle handle)
    {
        index_t index = _sparse[handle.get_index()];

        return index < _num_handles
            && _dense[index] == handle.get_index()
            && _versions[index] == handle.get_version();
    }

    void reset()
    {
        _num_handles = 0;
        for( index_t i = 0; i < M; i++ )
        {
            _dense[i] = i;
            _versions[i] = 0;
        }
    }

protected:
    index_t _num_handles = 0;
    index_t _dense[M];
    index_t _sparse[M];
    index_t _versions[M];
};

NS_LEMON_END