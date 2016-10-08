// @date 2016/07/21
// @author Mao Jingkai(oammix@gmail.com)

#include <codebase/memory/allocator.hpp>

#include <cstdlib>

NS_LEMON_BEGIN

const size_t FixedSizeAllocator::invalid = std::numeric_limits<size_t>::max();

FixedSizeAllocator::FixedSizeAllocator(size_t element_size, size_t chunk_size)
: _chunk_size(chunk_size), _first_free_block(invalid), _available(0), _total_elements(0)
{
    _element_size = element_size;
    if( _element_size < sizeof(size_t) )
        _element_size = sizeof(size_t);
}

FixedSizeAllocator::~FixedSizeAllocator()
{
    for( auto blocks : _chunks )
        ::free(blocks);
    _chunks.clear();
}

void* FixedSizeAllocator::malloc()
{
    if( _first_free_block == invalid )
    {
        ENSURE( _available == 0 );
        ENSURE( _total_elements < std::numeric_limits<size_t>::max() - _chunk_size);

        uint8_t* chunk = static_cast<uint8_t*>(::malloc(_chunk_size*_element_size));
        if( chunk == nullptr )
        {
            LOGW("failed to allocate memory[%d byte(s)] from system to initialize pool",
                _element_size*_chunk_size);
            return nullptr;
        }

        uint8_t* cursor = chunk;
        size_t offset = _chunk_size * _chunks.size();
        for( size_t i=1; i<_chunk_size; i++, cursor += _element_size )
            *(size_t*)cursor = offset + i;
        *(size_t*)cursor = invalid;

        _total_elements += _chunk_size;
        _available += _chunk_size;
        _first_free_block = offset;
        _chunks.push_back(chunk);
    }

    void* element = get_element(_first_free_block);
    ENSURE( element != nullptr );

    _first_free_block = *(size_t*)element;
    _available --;
    return element;
}

void FixedSizeAllocator::free(void* element)
{
    // find block index of the element
    size_t index = invalid;
    for( auto i = 0; i < _chunks.size(); i++ )
    {
        if( (size_t)element >= (size_t)_chunks[i] &&
            (size_t)element < (size_t)(_chunks[i]+_element_size*_chunk_size) )
        {
            index = i*_chunk_size + ((size_t)element - (size_t)_chunks[i])/_element_size;
            break;
        }
    }

    if( index == invalid )
    {
        LOGW("try to free element which does NOT belongs to this memory pool.");
        return;
    }

    // recycle this memory block
    *(size_t*)element = _first_free_block;
    _first_free_block = index;
    _available ++;
}

const size_t FixedBlockAllocator::invalid = std::numeric_limits<size_t>::max();

FixedBlockAllocator::FixedBlockAllocator(size_t element_size, size_t chunk_size)
: _element_size(element_size), _chunk_size(chunk_size), _available(chunk_size), _first_free_block(0)
{
    if( _element_size < sizeof(size_t) )
        _element_size = sizeof(size_t);

    uint8_t* chunk = static_cast<uint8_t*>(::malloc(_chunk_size*_element_size));

    ASSERT(chunk != nullptr, "failed to allocate memory[%d byte(s)] from system to initialize pool",
        _element_size*_chunk_size);

    _buffer.reset(chunk);

    uint8_t* cursor = chunk;
    for( size_t i = 1; i < _chunk_size; i ++, cursor += _element_size )
        *(size_t*)cursor = i;
    *(size_t*)cursor = invalid;
}

void* FixedBlockAllocator::malloc()
{
    if( _first_free_block == invalid )
        return nullptr;

    uint8_t* element = static_cast<uint8_t*>(&_buffer[_first_free_block*_element_size]);
    _first_free_block = *(size_t*)element;
    _available--;
    return element;
}

void FixedBlockAllocator::free(void* element)
{
    if( (size_t)element < (size_t)(&_buffer) || (size_t)element > (size_t)(&_buffer[_chunk_size+1]) )
    {
        LOGW("try to free element which does NOT belongs to this memory pool.");
        return;
    }

    size_t index = ((size_t)element - (size_t)(&_buffer[0]))/_element_size;
    *(size_t*)element = _first_free_block;
    _first_free_block = index;
    _available++;
}

NS_LEMON_END