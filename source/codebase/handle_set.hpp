// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/handle.hpp>

#include <vector>

NS_LEMON_BEGIN

// An handle contains an index and a version. when an handle is detroyed, the
// version associated with the index is incremented.
struct ReuseableHandleSet
{
    using index_t = Handle::index_t;

    // an iterator over all alive handle
    struct iterator : public std::iterator<std::forward_iterator_tag, Handle>
    {
        iterator(const ReuseableHandleSet& handles, Handle current = Handle())
        : _handles(handles), _current(current)
        {}

        iterator operator ++ (int dummy)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator ++ ()
        {
            _current = _handles.find_next_available(_current);
            return *this;
        }

        bool operator == (const iterator& rhs) const
        {
            return &_handles == &rhs._handles && _current == rhs._current;
        }

        bool operator != (const iterator& rhs) const
        {
            return !(*this == rhs);
        }

        Handle operator* () const
        {
            return _current;
        }

    protected:
        const ReuseableHandleSet& _handles;
        Handle _current;
    };

    Handle create()
    {
        if( _freeslots.size() != 0 )
        {
            index_t index = _freeslots.back();
            _freeslots.pop_back();

            ASSERT(_versions[index] < Handle::invalid - 1,
                "too much versions,"
                "please considering change the representation of Handle::index_t.");
            return Handle(index, ++_versions[index]);
        }

        _versions.push_back(1);
        ASSERT(_versions.size() < Handle::invalid,
            "too much handles,"
            "please considering change the representation of Handle::index_t.");

        return Handle(_versions.size()-1, 1);
    }

    bool is_valid(Handle handle) const
    {
        const index_t index = handle.get_index();
        const index_t version = handle.get_version();

        return index < _versions.size() && (_versions[index] & 0x1) == 1 && _versions[index] == version;
    }

    void free(Handle handle)
    {
        if( !is_valid(handle) )
            return;

        _versions[handle.get_index()]++;
        _freeslots.push_back(handle.get_index());
    }

    void clear()
    {
        _versions.clear();
        _freeslots.clear();
    }

    size_t size() const
    {
        return _versions.size() - _freeslots.size();
    }

    size_t capacity() const
    {
        return _versions.size();
    }

    index_t get_version(index_t index) const
    {
        return _versions.size() > index ? _versions[index] : Handle::invalid;
    }

    // returns an iterator referrring to the first alive handle in the handle set
    iterator begin() const
    {
        Handle handle = _versions.size() > 0 ? Handle(0, _versions[0]) : Handle();

        if( handle.is_valid() && !is_valid(handle) )
            handle = find_next_available(handle);

        return iterator(*this, handle);
    }

    // returns an iterator referrring to the past-the-end handle
    iterator end() const
    {
        return iterator(*this, Handle());
    }

protected:
    Handle find_next_available(Handle handle) const
    {
        for( index_t i = (handle.get_index() + 1); i < _versions.size(); i++ )
        {
            if( (_versions[i] & 0x1) == 1 )
                return Handle(i, _versions[i]);
        }

        return Handle();
    }

    // handle version numbers. incremented each time an handle is freed and created
    std::vector<index_t> _versions;
    // list of available handle slots
    std::vector<index_t> _freeslots;
};

NS_LEMON_END
