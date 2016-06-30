// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF POOL
template<typename T>
const T MemoryChunks<T>::invalid = std::numeric_limits<index_type>::max();

template<typename T>
MemoryChunks<T>::MemoryChunks(T element_size, T chunk_size)
: _chunk_size(chunk_size), _first_free_block(invalid), _available(0), _total_elements(0)
{
    static_assert( std::numeric_limits<T>::is_integer,
        "index type of MemoryChunks should be integer." );

    _element_size = element_size;
    if( _element_size < sizeof(T) )
        _element_size = sizeof(T);
}

template<typename T>
MemoryChunks<T>::~MemoryChunks()
{
    for( auto blocks : _chunks )
        ::free(blocks);
    _chunks.clear();
}

template<typename T>
INLINE T MemoryChunks<T>::size() const
{
    return _total_elements - _available;
}

template<typename T>
INLINE void* MemoryChunks<T>::get_element(T index)
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

template<typename T>
INLINE const void* MemoryChunks<T>::get_element(T index) const
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

template<typename T>
T MemoryChunks<T>::malloc()
{
    if( _first_free_block == invalid )
    {
        ENSURE( _available == 0 );
        ENSURE( _total_elements < std::numeric_limits<T>::max() - _chunk_size);

        uint8_t* chunk = static_cast<uint8_t*>(::malloc(_chunk_size*_element_size));
        if( chunk == nullptr )
        {
            LOGW("failed to allocate memory[%d byte(s)] from system to initialize pool",
                _element_size*_chunk_size);
            return invalid;
        }

        uint8_t* cursor = chunk;
        T offset = _chunk_size * _chunks.size();
        for( T i=1; i<_chunk_size; i++, cursor += _element_size )
            *(T*)cursor = offset + i;
        *(T*)cursor = invalid;

        _total_elements += _chunk_size;
        _available += _chunk_size;
        _first_free_block = offset;
        _chunks.push_back(chunk);
    }

    void* element = get_element(_first_free_block);
    ENSURE( element != nullptr );

    T result = _first_free_block;
    _first_free_block = *(T*)element;
    _available --;
    return result;
}

template<typename T>
void MemoryChunks<T>::free(T index)
{
    void* element = get_element(index);
    if( element == nullptr )
        return;

    *(T*)element = _first_free_block;
    _first_free_block = index;
    _available ++;
}