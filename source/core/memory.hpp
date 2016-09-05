// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>

NS_LEMON_CORE_BEGIN

// provides a resizable, semi-contiguous pool of memory for constructing
// objects in, which aims to provide cache-friendly iteration.
// lookups are O(1), appends/recycles are amortized O(1).
struct MemoryChunks
{
    using index_type = size_t;

    explicit MemoryChunks(index_type element_size, index_type chunk_size);
    virtual ~MemoryChunks();

    index_type size() const;
    index_type capacity() const;
    index_type chunk_size() const;
    index_type element_size() const;

    // returns a free memory block from pool
    void* malloc();
    // free and recycle specified memory block
    void  free(void*);

protected:
    const static size_t invalid;
    void* get_element(index_type);
    const void* get_element(index_type) const;

    std::vector<uint8_t*>   _chunks;
    index_type              _total_elements;
    index_type              _available;
    index_type              _element_size;      // size of each element block
    index_type              _chunk_size;        // number of blocks in every chunk
    index_type              _first_free_block;  // the index of first available chunk
};

// INCLUDED METHODS OF POOL
INLINE MemoryChunks::index_type MemoryChunks::size() const
{
    return _total_elements - _available;
}

INLINE MemoryChunks::index_type MemoryChunks::capacity() const
{
    return _total_elements;
}

INLINE MemoryChunks::index_type MemoryChunks::chunk_size() const
{
    return _chunk_size;
}

INLINE MemoryChunks::index_type MemoryChunks::element_size() const
{
    return _element_size;
}

INLINE void* MemoryChunks::get_element(index_type index)
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

INLINE const void* MemoryChunks::get_element(index_type index) const
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

NS_LEMON_CORE_END