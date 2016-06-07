// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF POOL

INLINE size_t MemoryChunks::size() const
{
    return m_size;
}

INLINE size_t MemoryChunks::capacity() const
{
    return m_capacity;
}

INLINE size_t MemoryChunks::chunks() const
{
    return m_blocks.size();
}

INLINE void MemoryChunks::resize(size_t n)
{
    if( n >= m_size )
    {
        if( n >= m_capacity ) reserve(n);
        m_size = n;
    }
}

INLINE void MemoryChunks::reserve(size_t n)
{
    while( m_capacity < n )
    {
        char* chunk = new char[m_element_size*m_chunk_size];
        m_blocks.push_back(chunk);
        m_capacity += m_chunk_size;
    }
}

INLINE void* MemoryChunks::get(size_t n)
{
    ENSURE( n < m_size );
    return m_blocks[n / m_chunk_size] + (n % m_chunk_size) * m_element_size;
}

INLINE const void* MemoryChunks::get(size_t n) const
{
    ENSURE( n < m_size );
    return m_blocks[n / m_chunk_size] + (n % m_chunk_size) * m_element_size;
}
