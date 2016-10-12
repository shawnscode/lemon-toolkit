// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <cstdlib>
#include <vector>
#include <type_traits>
#include <functional>
#include <limits>

NS_LEMON_BEGIN

// an memory pool is a specialized allocator that allocates memory in large chunks
// and deals them out in small slice, with O(1) amortized allocation and deallocation.
// its useful for a variety of cases:
// 1. reducing system call overhead when requesting multiple small memory blocks;
// 2. improving cache efficiency by keeping memory contiguous;
template<typename T, size_t Growth> struct MemoryPool
{
    using aligned_storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    // accquire a unused block of memory
    void* malloc();
    // recycle the memory to pool
    void free(void*);
    // destruct all objects and frees all chunks allocated
    void free_all();

    // returns the number of constructed objects
    size_t get_size() const;
    // returns the capacity of current pool
    size_t get_capacity() const;

protected:
    constexpr const static size_t invalid = std::numeric_limits<size_t>::max();

    size_t grow();

    std::vector<aligned_storage_t*> _chunks;
    size_t _available;
    size_t _first_free_block;
};

template<typename T, size_t Growth> size_t MemoryPool<T, Growth>::get_size() const
{
    return _chunks.size() * Growth - _available;
}

template<typename T, size_t Growth> size_t MemoryPool<T, Growth>::get_capacity() const
{
    return _chunks.size() * Growth;
}

template<typename T, size_t Growth> void* MemoryPool<T, Growth>::malloc()
{
    if( _first_free_block == invalid )
    {
        // trying to grow this pool if we are out of free block
        _first_free_block = grow();
        if( _first_free_block == invalid )
        {
            LOGW("failed to allocate memory[%d byte(s)] from system to grow pool",
                sizeof(aligned_storage_t)*Growth);
            return nullptr;
        }
    }

    auto block = _chunks[_first_free_block/_chunks.size()]+(_first_free_block%_chunks.size());
    _first_free_block = *(size_t*)block;
    _available --;

    return static_cast<void*>(block);
}

template<typename T, size_t Growth> void MemoryPool<T, Growth>::free(void* block)
{
    // find block index of the element
    size_t index = invalid;
    for( size_t i = 0; i < _chunks.size(); i++ )
    {
        if( (size_t)block >= (size_t)_chunks[i] && (size_t)block < (size_t)(_chunks[i]+Growth) )
        {
            index = i*_chunks.size() + ((size_t)block - (size_t)_chunks[i])/sizeof(aligned_storage_t);
            break;
        }
    }

    if( index == invalid )
    {
        LOGW("try to free block which does NOT belongs to this memory pool.");
        return;
    }

    // recycle this memory block, add it to the first of free list
    *(size_t*)block = _first_free_block;
    _first_free_block = index;
    _available ++;
}

template<typename T, size_t Growth> void MemoryPool<T, Growth>::free_all()
{
    // returns allocated chunk to system
    for( auto chunk : _chunks )
        ::free(chunk);

    _chunks.clear();
    _available = 0;
    _first_free_block = invalid;
}

template<typename T, size_t Growth> size_t MemoryPool<T, Growth>::grow()
{
    const auto block_size = sizeof(aligned_storage_t);

    auto chunk = static_cast<size_t*>(::malloc(Growth*block_size));
    if( chunk == nullptr )
        return invalid;

    auto iterator = chunk;
    auto offset = Growth * _chunks.size();
    for( size_t i = 1; i < Growth; i++, iterator += block_size )
        *iterator = offset + i;
    *iterator = invalid;

    _available += Growth;
    _chunks.push_back(static_cast<aligned_storage_t*>(chunk));
    return offset;
}

NS_LEMON_END