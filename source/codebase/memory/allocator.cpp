// @date 2016/07/21
// @author Mao Jingkai(oammix@gmail.com)

#include <codebase/memory/allocator.hpp>

NS_LEMON_BEGIN

const FixedSizeAllocator::index_type FixedSizeAllocator::invalid = std::numeric_limits<index_type>::max();

FixedSizeAllocator::FixedSizeAllocator(index_type element_size, index_type chunk_size)
: _chunk_size(chunk_size), _first_free_block(invalid), _available(0), _total_elements(0)
{
    _element_size = element_size;
    if( _element_size < sizeof(index_type) )
        _element_size = sizeof(index_type);
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
        ENSURE( _total_elements < std::numeric_limits<index_type>::max() - _chunk_size);

        uint8_t* chunk = static_cast<uint8_t*>(::malloc(_chunk_size*_element_size));
        if( chunk == nullptr )
        {
            LOGW("failed to allocate memory[%d byte(s)] from system to initialize pool",
                _element_size*_chunk_size);
            return nullptr;
        }

        uint8_t* cursor = chunk;
        index_type offset = _chunk_size * _chunks.size();
        for( index_type i=1; i<_chunk_size; i++, cursor += _element_size )
            *(index_type*)cursor = offset + i;
        *(index_type*)cursor = invalid;

        _total_elements += _chunk_size;
        _available += _chunk_size;
        _first_free_block = offset;
        _chunks.push_back(chunk);
    }

    void* element = get_element(_first_free_block);
    ENSURE( element != nullptr );

    _first_free_block = *(index_type*)element;
    _available --;
    return element;
}

void FixedSizeAllocator::free(void* element)
{
    // find block index of the element
    index_type index = invalid;
    for( auto i = 0; i < _chunks.size(); i++ )
    {
        if( (index_type)element >= (index_type)_chunks[i] &&
            (index_type)element < (index_type)(_chunks[i]+_element_size*_chunk_size) )
        {
            index = i*_chunk_size + ((index_type)element - (index_type)_chunks[i])/_element_size;
            break;
        }
    }

    if( index == invalid )
    {
        LOGW("try to free element which does NOT belongs to this memory pool.");
        return;
    }

    // recycle this memory block
    *(index_type*)element = _first_free_block;
    _first_free_block = index;
    _available ++;
}

NS_LEMON_END