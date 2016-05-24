#include "pool.hpp"

NS_FLOW2D_BEGIN

GenericPool::GenericPool(size_t element_size, size_t chunk_size)
: m_element_size(element_size), m_chunk_size(chunk_size), m_capacity(0), m_size(0) {}

GenericPool::~GenericPool()
{
    for( char* ptr : m_blocks ) delete[] ptr;
}

NS_FLOW2D_END