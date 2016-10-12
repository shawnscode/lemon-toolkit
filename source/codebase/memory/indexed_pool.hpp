// @date 2016/10/11
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <codebase/handle.hpp>
#include <codebase/handle_set.hpp>
#include <codebase/memory/memory_pool.hpp>

NS_LEMON_BEGIN

// an indexed memory pool provides O(1) amortized allocation/deallocation/lookup
// of fixed size memory block associated with handle.
template<typename T, size_t Growth> struct IndexedMemoryPool : protected MemoryPool<T, Growth>
{
    // accquire a unused block of memory from pool, returns handle associated with it
    Handle malloc();
    // returns memory block associated with handle
    void* get(Handle handle);
    // returns memory block to pool and invalidate current handle
    void free(Handle);
    // destruct all objects/handles and frees all allocated chunks
    void free_all();

    // returns the iterator referrring to the first alive handle
    ReuseableHandleSet::iterator begin() const;
    // returns an iterator referrring to the past-the-end handle
    ReuseableHandleSet::iterator end() const;

protected:
    ReuseableHandleSet _handles;
    std::vector<void*> _table;
};

template<typename T, size_t Growth> Handle IndexedMemoryPool<T, Growth>::malloc()
{
    auto block = MemoryPool<T, Growth>::malloc();
    if( block == nullptr )
        return Handle();

    auto handle = _handles.create();
    if( _table.size() <= handle.get_index() )
        _table.resize(handle.get_index()+1, nullptr);

    _table[handle.get_index()] = block();
    return handle;
}

template<typename T, size_t Growth> void* IndexedMemoryPool<T, Growth>::get(Handle handle)
{
    return _handles.is_valid(handle) ? _table[handle.get_index()] : nullptr;
}

template<typename T, size_t Growth> void IndexedMemoryPool<T, Growth>::free(Handle handle)
{
    if( !_handles.is_valid(handle) )
        return;

    MemoryPool<T, Growth>::free(_table[handle.get_index()]);
    _handles.free(handle);
    _table[handle.get_index()] = nullptr;
}

template<typename T, size_t Growth> void IndexedMemoryPool<T, Growth>::free_all()
{
    MemoryPool<T, Growth>::free_all();
    _handles.clear();
    _table.clear();
}

template<typename T, size_t Growth> ReuseableHandleSet::iterator IndexedMemoryPool<T, Growth>::begin() const
{
    return _handles.begin();
}

template<typename T, size_t Growth> ReuseableHandleSet::iterator IndexedMemoryPool<T, Growth>::end() const
{
    return _handles.end();
}

NS_LEMON_END