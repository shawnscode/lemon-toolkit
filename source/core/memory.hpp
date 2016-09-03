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

template<typename T> struct ObjectChunks : public MemoryChunks
{
    // allocate and construct a instance of T
    template<typename ... Args> T* spawn(Args&&...);
    // destruct and recycle the memory useage
    void dispose(T*);
};

template<typename T, typename I, size_t DS> struct IndexedObjectChunks : public MemoryChunks
{
    using index_type = I;
    explicit IndexedObjectChunks(index_type chunk_size);
    virtual ~IndexedObjectChunks();

    // allocate and construct a instance of T
    template<typename ... Args> T* spawn(I, Args&&...);
    // destruct and recycle the memory useage
    void dispose(I);
    // returns instance of T identified by index
    T* find(I);

protected:
    bool            _fallback = false;
    size_t          _redirect_index[DS] = {};
    size_t          _top = DS;
    std::vector<T*> _objects;
};

#include <core/memory.inl>
NS_LEMON_CORE_END