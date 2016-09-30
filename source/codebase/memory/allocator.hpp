// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <vector>

NS_LEMON_BEGIN

// provides a 7 for constructing
// objects in, which aims to provide cache-friendly iteration.
// lookups are O(1), appends/recycles are amortized O(1).
struct FixedSizeAllocator
{
    explicit FixedSizeAllocator(size_t element_size, size_t chunk_size);
    virtual ~FixedSizeAllocator();

    size_t size() const;
    size_t capacity() const;
    size_t chunk_size() const;
    size_t element_size() const;

    // returns a free memory block from pool
    void* malloc();
    // free and recycle specified memory block
    void  free(void*);

protected:
    const static size_t invalid;

    void* get_element(size_t);
    const void* get_element(size_t) const;

    std::vector<uint8_t*> _chunks;
    size_t _total_elements;
    size_t _available;
    size_t _element_size;      // size of each element block
    size_t _chunk_size;        // number of blocks in every chunk
    size_t _first_free_block;  // the index of first available chunk
};

// INCLUDED METHODS OF POOL
INLINE size_t FixedSizeAllocator::size() const
{
    return _total_elements - _available;
}

INLINE size_t FixedSizeAllocator::capacity() const
{
    return _total_elements;
}

INLINE size_t FixedSizeAllocator::chunk_size() const
{
    return _chunk_size;
}

INLINE size_t FixedSizeAllocator::element_size() const
{
    return _element_size;
}

INLINE void* FixedSizeAllocator::get_element(size_t index)
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

INLINE const void* FixedSizeAllocator::get_element(size_t index) const
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

NS_LEMON_END