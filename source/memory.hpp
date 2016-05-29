// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include "flow2d.hpp"
#include <vector>

NS_FLOW2D_BEGIN

struct MemoryChunks
{
    explicit MemoryChunks(size_t element_size, size_t chunk_size);
    virtual ~MemoryChunks();

    size_t      size() const;
    size_t      capacity() const;
    size_t      chunks() const;

    void*       get(size_t);
    const void* get(size_t) const;
    void        resize(size_t);
    void        reserve(size_t);

protected:
    std::vector<char *> m_blocks;
    size_t              m_element_size;
    size_t              m_chunk_size;
    size_t              m_size = 0;
    size_t              m_capacity = 0;
};

struct ObjectChunks : public MemoryChunks
{
    ObjectChunks(size_t element_size, size_t chunk_size)
    : MemoryChunks(element_size, chunk_size) {}

    virtual void construct_from(size_t, size_t) = 0;
    virtual void destruct(size_t) = 0;
};

template<typename T, size_t ChunkSize = 1024>
struct ObjectChunksTrait : public ObjectChunks
{
    ObjectChunksTrait() : ObjectChunks(sizeof(T), ChunkSize) {}

    template<typename ... Args> T* construct(size_t n, Args&& ... args)
    {
        auto chunk = get(n);
        ::new(chunk) T(std::forward<Args>(args) ...);
        return static_cast<T*>(chunk);
    }

    virtual void destruct(size_t n) override
    {
        auto chunk = static_cast<T*>(get(n));
        chunk->~T();
    }

    virtual void construct_from(size_t from, size_t to) override
    {
        ::new(get(to)) T( *static_cast<T*>(get(from)) );
    }
};

#include "memory.inl"
NS_FLOW2D_END