// INCLUDED METHODS OF POOL

INLINE size_t GenericPool::size() const
{
    return m_size;
}

INLINE size_t GenericPool::capacity() const
{
    return m_capacity;
}

INLINE size_t GenericPool::chunks() const
{
    return m_blocks.size();
}

INLINE void GenericPool::expand(size_t n)
{
    if( n >= m_size )
    {
        if( n >= m_capacity ) reserve(n);
        m_size = n;
    }
}

INLINE void GenericPool::reserve(size_t n)
{
    while( m_capacity < n )
    {
        char* chunk = new char[m_element_size*m_chunk_size];
        m_blocks.push_back(chunk);
        m_capacity += m_chunk_size;
    }
}

INLINE void* GenericPool::get(size_t n)
{
    assert( n < m_size );
    return m_blocks[n / m_chunk_size] + (n % m_chunk_size) * m_element_size;
}

INLINE const void* GenericPool::get(size_t n) const
{
    assert( n < m_size );
    return m_blocks[n / m_chunk_size] + (n % m_chunk_size) * m_element_size;
}
