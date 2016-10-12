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
struct IndexedMemoryPool : protected MemoryPool
{
    IndexedMemoryPool(size_t block_size, size_t chunk_size) : MemoryPool(block_size, chunk_size) {}

    // accquire a unused block of memory from pool, returns handle associated with it
    Handle malloc();
    // returns memory block associated with handle
    void* get(Handle handle);
    // returns memory block to pool and invalidate current handle
    void free(Handle);
    // destruct all objects/handles and frees all allocated chunks
    void free_all();
    // returns true if handle is valid
    bool is_valid(Handle) const;

    // returns size of allocated blocks
    size_t size() const;
    // returns capacity of this memory pool
    size_t capacity() const;

    // returns the iterator referrring to the first alive handle
    ReuseableHandleSet::iterator begin() const;
    // returns an iterator referrring to the past-the-end handle
    ReuseableHandleSet::iterator end() const;

protected:
    ReuseableHandleSet _handles;
    std::vector<void*> _table;
};

template<typename T, size_t Growth> struct IndexedMemoryPoolT : public IndexedMemoryPool
{
    using aligned_storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    IndexedMemoryPoolT() : IndexedMemoryPool(sizeof(aligned_storage_t), Growth)
    {}

    T* get_t(Handle handle) { return static_cast<T*>(get(handle)); }
};

INLINE bool IndexedMemoryPool::is_valid(Handle handle) const
{
    return _handles.is_valid(handle);
}

INLINE size_t IndexedMemoryPool::size() const
{
    return MemoryPool::size();
}

INLINE size_t IndexedMemoryPool::capacity() const
{
    return MemoryPool::capacity();
}

INLINE ReuseableHandleSet::iterator IndexedMemoryPool::begin() const
{
    return _handles.begin();
}

INLINE ReuseableHandleSet::iterator IndexedMemoryPool::end() const
{
    return _handles.end();
}

NS_LEMON_END