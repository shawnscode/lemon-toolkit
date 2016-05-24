#include "memory.hpp"

NS_FLOW2D_BEGIN

MemoryChunks::MemoryChunks(size_t element_size, size_t chunk_size)
: m_element_size(element_size), m_chunk_size(chunk_size)
{}

MemoryChunks::~MemoryChunks()
{
    for( char* ptr : m_blocks ) delete[] ptr;
}

NS_FLOW2D_END