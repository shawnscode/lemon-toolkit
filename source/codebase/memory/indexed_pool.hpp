// @date 2016/10/11
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <codebase/handle.hpp>
#include <codebase/handle_set.hpp>
#include <codebase/memory/allocator.hpp>

#include <functional>

NS_LEMON_BEGIN

struct IndexedMemoryPool
{
    IndexedMemoryPool(size_t element_size, size_t chunk_size);

    // accquire a unused block of memory from pool, returns handle associated with it
    Handle malloc();

    // returns memory block to pool and invalidate current handle
    void free(Handle);

    // returns memory block associated with handle
    void* get(Handle handle)
    {
        return _handles.is_valid(handle) ? _table[handle.get_index()] : nullptr;
    }

    const void* get(Handle handle) const
    {
        return _handles.is_valid(handle) ? _table[handle.get_index()] : nullptr;
    }

    void clear()
    {
        _allocator.clear();
        _handles.clear();
        _table.clear();
    }

    ReuseableHandleSet::iterator begin() const
    {
        return _handles.begin();
    }

    ReuseableHandleSet::iterator end() const
    {
        return _handles.end();
    }

protected:
    FixedSizeAllocator _allocator;
    ReuseableHandleSet _handles;
    std::vector<void*> _table;
};

struct IndexedObjectPool : public IndexedMemoryPool
{
    using callback = std::function<void(void*)>;

    IndexedObjectPool(size_t element_size, size_t chunk_size, const callback& ctor, const callback& dtor);
    ~IndexedObjectPool();

    // accquire a unused block of memory and construct it with constructor
    Handle malloc();
    // destruct object and recycle memory
    void free(Handle);

protected:
    callback _constructor;
    callback _destructor;
};

NS_LEMON_END