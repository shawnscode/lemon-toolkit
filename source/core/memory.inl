// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF POOL

INLINE size_t MemoryPool::available() const
{
    size_t available = 0;
    for( auto cursor = m_first_chunk; cursor; cursor = cursor->next )
        available += cursor->available;
    return available;
}

INLINE size_t MemoryPool::capacity() const
{
    size_t capacity = 0;
    for( auto cursor = m_first_chunk; cursor; cursor = cursor->next )
        capacity += cursor->size;
    return capacity;
}

INLINE size_t MemoryPool::chunks() const
{
    size_t chunks = 0;
    for( auto cursor = m_first_chunk; cursor; cursor = cursor->next )
        chunks ++;
    return chunks;
}

INLINE void* MemoryPool::get_first_block_of_chunk(MemoryChunk* chunk)
{
    return (void*)((uint8_t*)chunk + sizeof(MemoryChunk));
}

INLINE void* MemoryPool::get_end_of_chunk(MemoryChunk* chunk)
{
    return (void*)((uint8_t*)get_first_block_of_chunk(chunk) + m_element_size * chunk->size);
}

INLINE bool MemoryPool::is_block_in_chunk(MemoryChunk* chunk, void* block)
{
    return
        ((unsigned long)block >= (unsigned long)get_first_block_of_chunk(chunk)) &&
        ((unsigned long)block < (unsigned long)get_end_of_chunk(chunk));
}

template<typename T>
ObjectChunksTrait<T>::ObjectChunksTrait(size_t chunk_size, size_t grow_chunk_size)
: ObjectChunks(sizeof(T), chunk_size, grow_chunk_size)
{}

template<typename T>
ObjectChunksTrait<T>::~ObjectChunksTrait()
{
    for( auto cursor : m_slots )
        cursor.second->~T();
}

template<typename T>
template<typename ... Args>
INLINE T* ObjectChunksTrait<T>::construct(size_t index, Args && ... args)
{
    ENSURE( m_slots.find(index) == m_slots.end() );

    auto block = malloc();
    ::new(block) T(std::forward<Args>(args) ...);

    m_slots[index] = static_cast<T*>(block);
    return static_cast<T*>(block);
}

template<typename T>
void ObjectChunksTrait<T>::construct_from(size_t to, size_t from)
{
    ENSURE( m_slots.find(from) != m_slots.end() );
    construct( to, std::forward<const T&>(*get(from)) );
}

template<typename T>
void ObjectChunksTrait<T>::destruct(size_t index)
{
    auto cursor = m_slots.find(index);
    if( cursor == m_slots.end() )
    {
        LOGW("try to destruct a non-exist object.");
        return;
    }

    cursor->second->~T();
    free(cursor->second);
    m_slots.erase(cursor);
}

template<typename T>
INLINE T* ObjectChunksTrait<T>::get(size_t index)
{
    auto cursor = m_slots.find(index);
    if( cursor == m_slots.end() ) return nullptr;

    return cursor->second;
}

template<typename T>
INLINE const T* ObjectChunksTrait<T>::get(size_t index) const
{
    auto cursor = m_slots.find(index);
    if( cursor == m_slots.end() ) return nullptr;

    return cursor->second;
}