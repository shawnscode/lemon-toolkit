// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/handle.hpp>

#include <array>
#include <mutex>
#include <vector>

NS_LEMON_BEGIN

template<typename T>
struct HashSetIterator : public std::iterator<std::forward_iterator_tag, Handle>
{
    HashSetIterator(const T& handles, Handle position);

    HashSetIterator operator ++ (int dummy);
    HashSetIterator& operator ++ ();

    bool operator == (const HashSetIterator& rhs) const;
    bool operator != (const HashSetIterator& rhs) const;

    Handle operator* () const;

protected:
    const T& _handles;
    Handle _position;
};

/**
 * @brief      A thread-safe handle creation and recycle manager.
 *
 * @tparam     S     The max size of available handles, should be less than Handle::invalid.
 */
template<size_t N> struct HandleSet
{
    static_assert(N < Handle::invalid,
        "The max size of handle set should be less than Handle::invalid.");

    using mutex_t = std::mutex;
    using index_t = Handle::index_t;
    using array_t = std::array<index_t, N>;

public:
    HandleSet();

    /**
     * @brief      Create a unique handle and mark it as alive internally.
     *
     * @return     Returns alive handle if create successfully, invalid otherwise.
     */
    Handle create();

    /**
     * @brief      Determines if alive.
     *
     * @param[in]  handle  The handle
     *
     * @return     True if alive, False otherwise.
     */
    bool is_alive(Handle handle) const;


    /**
     * @brief      Recycle this handle index, and mark it's version as dead.
     *
     * @param[in]  handle  The handle
     *
     * @return     True if freed, False otherwise
     */
    bool free(Handle handle);

    /**
     * @brief      Reset this handle pool to initial state.
     */
    void clear();


public:
    using const_iterator_t = HashSetIterator<HandleSet<N>>;

    // returns an iterator referrring to the first alive handle in the handle set
    // 
    
    /**
     * @brief      Create an constant iterator referring to the first alive handle.
     *
     * @return     Returns created iterator.
     */
    const_iterator_t begin() const;

    /**
     * @brief      Create an constant iterator referring to the past-the-end handle.
     *
     * @return     Returns created iterator.
     */
    const_iterator_t end() const;

protected:
    friend struct HashSetIterator<HandleSet<N>>;
    Handle find_next_available(Handle handle) const;

    mutex_t _mutex;
    index_t _available;
    array_t _versions;
    array_t _freeslots;
};

template<size_t N> INLINE HandleSet<N>::HandleSet()
{
    clear();
}

template<size_t N> INLINE Handle HandleSet<N>::create()
{
    std::unique_lock<std::mutex> lock(_mutex);

    if( _available > 0 )
    {
        index_t index = _freeslots[--_available];
        return Handle(index, ++_versions[index]);
    }

    return Handle();
}

template<size_t N> INLINE bool HandleSet<N>::is_alive(Handle handle) const
{
    auto index = handle.get_index();
    auto version = handle.get_version();

    return index < N && (_versions[index] & 0x1) == 1 && _versions[index] == version;
}

template<size_t N> INLINE bool HandleSet<N>::free(Handle handle)
{
    std::unique_lock<std::mutex> lock(_mutex);

    if( !is_alive(handle) )
        return false;

    _versions[handle.get_index()]++;
    _freeslots[_available++] = handle.get_index();
    return true;
}

template<size_t N> INLINE void HandleSet<N>::clear()
{
    std::unique_lock<std::mutex> lock(_mutex);

    memset(_versions.data(), 0, sizeof(index_t)*N);

    for( index_t i = 0; i < N; i++ )
        _freeslots[i] = N-i-1;

    _available = N;
}

template<size_t N> INLINE typename HandleSet<N>::const_iterator_t HandleSet<N>::begin() const
{
    Handle handle = Handle(0, _versions[0]);
    return const_iterator_t(*this, is_alive(handle) ? handle : find_next_available(handle));
}

template<size_t N> INLINE typename HandleSet<N>::const_iterator_t HandleSet<N>::end() const
{
    return const_iterator_t(*this, Handle());
}

template<size_t N> INLINE Handle HandleSet<N>::find_next_available(Handle handle) const
{
    for( index_t i = (handle.get_index() + 1); i < N; i++ )
    {
        if( (_versions[i] & 0x1) == 1 )
            return Handle(i, _versions[i]);
    }

    return Handle();
}

template<typename T> INLINE HashSetIterator<T>::HashSetIterator(const T& handles, Handle position)
: _handles(handles), _position(position)
{}

template<typename T> INLINE HashSetIterator<T> HashSetIterator<T>::operator ++ (int dummy)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T> INLINE HashSetIterator<T>& HashSetIterator<T>::operator ++ ()
{
    _position = _handles.find_next_available(_position);
    return *this;
}

template<typename T> INLINE bool HashSetIterator<T>::operator == (const HashSetIterator& rhs) const
{
    return &_handles == &rhs._handles && _position == rhs._position;
}

template<typename T> INLINE bool HashSetIterator<T>::operator != (const HashSetIterator& rhs) const
{
    return !(*this == rhs);
}

template<typename T> INLINE Handle HashSetIterator<T>::operator* () const
{
    return _position;
}

struct ReuseableHandleSet
{
    using index_t = Handle::index_t;

    // an iterator over all alive handle
    template<typename T>
    struct iterator : public std::iterator<std::forward_iterator_tag, Handle>
    {
        iterator(const T& handles, Handle current = Handle())
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
        const T& _handles;
        Handle _current;
    };

    using iterator_t = iterator<ReuseableHandleSet>;

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

    // returns an iterator referrring to the first alive handle in the handle set
    iterator_t begin() const
    {
        Handle handle = _versions.size() > 0 ? Handle(0, _versions[0]) : Handle();

        if( handle.is_valid() && !is_valid(handle) )
            handle = find_next_available(handle);

        return iterator_t(*this, handle);
    }

    // returns an iterator referrring to the past-the-end handle
    iterator_t end() const
    {
        return iterator_t(*this, Handle());
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

// fixed-size version of ReuseableHandleSet
template<size_t N> struct FixedReuseableHandleSet
{
    static_assert( N < Handle::invalid, "" );

    using index_t = ReuseableHandleSet::index_t;
    using iterator_t = ReuseableHandleSet::iterator<FixedReuseableHandleSet>;

    FixedReuseableHandleSet()
    {
        clear();
    }

    Handle create()
    {
        if( _available > 0 )
        {
            index_t index = _freeslots[--_available];
            return Handle(index, ++_versions[index]);
        }

        return Handle();
    }

    bool is_valid(Handle handle) const
    {
        const index_t index = handle.get_index();
        const index_t version = handle.get_version();

        return index < N && (_versions[index] & 0x1) == 1 && _versions[index] == version;
    }

    void free(Handle handle)
    {
        if( !is_valid(handle) )
            return;

        _versions[handle.get_index()]++;
        _freeslots[_available++] = handle.get_index();
    }

    void clear()
    {
        memset(_versions, 0, sizeof(index_t)*N);

        for( index_t i = 0; i < N; i++ )
            _freeslots[i] = i;

        _available = N;
    }

    // returns an iterator referrring to the first alive handle in the handle set
    iterator_t begin() const
    {
        Handle handle = Handle(0, _versions[0]);
        return iterator_t(*this, is_valid(handle) ? handle : find_next_available(handle));
    }

    // returns an iterator referrring to the past-the-end handle
    iterator_t end() const
    {
        return iterator_t(*this, Handle());
    }

protected:
    Handle find_next_available(Handle handle) const
    {
        for( index_t i = (handle.get_index() + 1); i < N; i++ )
        {
            if( (_versions[i] & 0x1) == 1 )
                return Handle(i, _versions[i]);
        }

        return Handle();
    }

    index_t _available;
    index_t _versions[N];
    index_t _freeslots[N];
};

NS_LEMON_END
