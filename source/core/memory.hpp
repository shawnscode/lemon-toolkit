// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>

NS_FLOW2D_BEGIN

struct MemoryChunk
{
    uint16_t        size;               // total size of blocks in this chunk, related to number of blocks
    uint16_t        available;          // how many blocks available in this chunk
    uint16_t        first_free_block;   // the index of first available chunk
    MemoryChunk*    next;
};

struct MemoryPool
{
    explicit MemoryPool(size_t element_size, size_t chunk_size, size_t grow_chunk_size);
    virtual ~MemoryPool();

    size_t  available() const;
    size_t  capacity() const;
    size_t  chunks() const;

    void*   malloc();
    void    free(void*);

protected:
    MemoryChunk*    allocate_new_chunk(size_t);
    void*           get_first_block_of_chunk(MemoryChunk*);
    void*           get_end_of_chunk(MemoryChunk*);
    bool            is_block_in_chunk(MemoryChunk*, void*);

    uint16_t        m_element_size;     // size of each block in pool
    uint16_t        m_chunk_size;       // number of blocks in first chunk
    uint16_t        m_grow_chunk_size;  // when first chunk is full, extend a new chunk have such blocks
    MemoryChunk*    m_first_chunk;      // pointer to first chunk
};

struct ObjectChunks : public MemoryPool
{
    explicit ObjectChunks(size_t element_size, size_t chunk_size, size_t grow_chunk_size)
    : MemoryPool(element_size, chunk_size, grow_chunk_size) {}

    virtual ~ObjectChunks() {}
    virtual void construct_from(size_t, size_t) = 0;
    virtual void destruct(size_t) = 0;
};

template<typename T>
struct ObjectChunksTrait : public ObjectChunks
{
    explicit ObjectChunksTrait(size_t chunk_size, size_t grow_chunk_size);
    virtual ~ObjectChunksTrait();

    template<typename ... Args> T* construct(size_t, Args && ... args);
    void construct_from(size_t, size_t) override;
    void destruct(size_t) override;

    T*          get(size_t);
    const T*    get(size_t) const;

protected:
    std::unordered_map<size_t, T*> m_slots;
};

#include <core/memory.inl>
NS_FLOW2D_END