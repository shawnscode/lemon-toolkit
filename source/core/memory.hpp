// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <cstdlib>

NS_FLOW2D_BEGIN

// provides a resizable, semi-contiguous pool of memory for constructing
// objects in, which aims to provide cache-friendly iteration.
// lookups are O(1), appends are amortized O(1).
template<typename IndexType = size_t> struct MemoryChunks
{
    using index_type = IndexType;
    const static index_type invalid;

    explicit MemoryChunks(index_type element_size, index_type chunk_size);
    virtual ~MemoryChunks();

    index_type  size() const;

    index_type  malloc();
    void        free(index_type);

    void*       get_element(index_type);
    const void* get_element(index_type) const;

protected:
    std::vector<uint8_t*>   _chunks;
    index_type              _total_elements;
    index_type              _available;
    index_type              _element_size;      // size of each element block
    index_type              _chunk_size;        // number of blocks in every chunk
    index_type              _first_free_block;  // the index of first available chunk
};

#include <core/memory.inl>
NS_FLOW2D_END