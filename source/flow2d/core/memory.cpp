// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/core/memory.hpp>
#include <cstdlib>

NS_FLOW2D_BEGIN

MemoryPool::MemoryPool(size_t element_size, size_t chunk_size, size_t grow_chunk_size)
: m_chunk_size(chunk_size), m_grow_chunk_size(grow_chunk_size), m_first_chunk(nullptr)
{
    m_element_size = element_size;
    if( m_element_size < sizeof(uint16_t) )
        m_element_size = sizeof(uint16_t);
}

MemoryPool::~MemoryPool()
{
    for( auto cursor = m_first_chunk; cursor != nullptr; )
    {
        auto tmp = cursor;
        cursor = cursor->next;
        ::free(tmp);
    }

    m_first_chunk = nullptr;
}

MemoryChunk* MemoryPool::allocate_new_chunk(size_t chunk_size)
{
    ENSURE( chunk_size < std::numeric_limits<uint16_t>::max() );

    auto chunk = (MemoryChunk*) ::malloc(sizeof(MemoryChunk) + chunk_size * m_element_size);
    if( chunk == nullptr )
        return nullptr;

    chunk->available        = chunk_size;
    chunk->size             = chunk_size;
    chunk->first_free_block = 0;
    chunk->next             = nullptr;

    // initialize this chunk, and create idle index
    void* block = get_first_block_of_chunk(chunk);
    for( size_t i=1; i<chunk_size; i++, block = (void*)((uint8_t*)block + m_element_size) )
        *(uint16_t*)block = i;

    return chunk;
}

void* MemoryPool::malloc()
{
    void* block = nullptr;
    auto chunk = m_first_chunk;

    // if no chunk in pool, create it
    if( chunk == nullptr )
    {
        m_first_chunk = allocate_new_chunk(m_chunk_size);
        chunk = m_first_chunk;

        if( chunk == nullptr )
        {
            LOGW("allocate memory[%d byte(s)] from system to initialize pool failed",
                m_element_size*m_chunk_size);
            return nullptr;
        }
    }

    // find a chunk which have available blocks
    while( chunk && !chunk->available ) chunk = chunk->next;

    if( chunk != nullptr )
    {
        block = get_first_block_of_chunk(chunk);
        block = (uint8_t*)block + chunk->first_free_block * m_element_size;
        chunk->first_free_block = *(uint16_t*)block;
        chunk->available --;
    }
    else
    {
        if( !m_grow_chunk_size )
        {
            LOGW("no blocks in pool and not allowed to automatically grow");
            return nullptr;
        }

        chunk = allocate_new_chunk(m_grow_chunk_size);
        if( chunk == nullptr )
        {
            LOGW("allocate memory[%d byte(s)] from system to extend pool failed",
                m_element_size*m_grow_chunk_size);
            return nullptr;
        }

        chunk->next = m_first_chunk;
        m_first_chunk = chunk;

        block = get_first_block_of_chunk(chunk);
        ENSURE( block != nullptr );
        chunk->first_free_block = *(uint16_t*)block;
        chunk->available --;
    }

    return block;
}

INLINE static bool is_chunk_empty(MemoryChunk* chunk)
{
    return chunk->available == chunk->size;
}

INLINE static bool is_chunk_full(MemoryChunk* chunk)
{
    return chunk->available == 0;
}

void MemoryPool::free(void* block)
{
    MemoryChunk* chunk = m_first_chunk;
    MemoryChunk* prev_chunk = nullptr;

    while( chunk && !is_block_in_chunk(chunk, block) )
    {
        prev_chunk = chunk;
        chunk = chunk->next;
    }

    if( nullptr == chunk )
    {
        LOGW("not found this memory block in pool, failed to free it");
        return;
    }

    chunk->available ++;
    *(uint16_t*)block = chunk->first_free_block;
    chunk->first_free_block = (uint16_t)
        (((unsigned long)block - (unsigned long)get_first_block_of_chunk(chunk)) / m_element_size);


    if( is_chunk_empty(chunk) )
    {
        if( chunk != m_first_chunk )
        {
            prev_chunk->next = chunk->next;
            chunk->next = m_first_chunk;
            m_first_chunk = chunk;
        }

        if( m_first_chunk->next && !is_chunk_full(m_first_chunk->next) )
        {
            m_first_chunk = chunk->next;
            ::free(chunk);
        }
    }
}

NS_FLOW2D_END