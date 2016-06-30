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

/// INCLUDED METHODS OF OBJECT CHUNKS
template<typename T, typename I>
ObjectChunksTrait<T, I>::~ObjectChunksTrait<T, I>()
{
    for( index_type i = 0; i < _memory_indices.size(); i++ )
        dispose(i);
}


template<typename T, typename I>
INLINE void ObjectChunksTrait<T, I>::resize(I size)
{
    _memory_indices.resize(size, MemoryChunks<I>::invalid);
}

template<typename T, typename I>
INLINE I ObjectChunksTrait<T, I>::capacity() const
{
    return _memory_indices.size();
}

template<typename T, typename I>
template<typename ... Args>
T* ObjectChunksTrait<T, I>::spawn(I index, Args&& ... args)
{
    ENSURE( _memory_indices[index] == MemoryChunks<I>::invalid );

    I position = MemoryChunks<I>::malloc();
    if( position == MemoryChunks<I>::invalid )
        return nullptr;

    void* object = MemoryChunks<I>::get_element(position);
    ::new(object) T(std::forward<Args>(args)...);

    _memory_indices[index] = position;
    return static_cast<T*>(object);
}

template<typename T, typename I>
void ObjectChunksTrait<T, I>::clone(I dest, I source)
{
    ENSURE( source < _memory_indices.size() && dest < _memory_indices.size() );

    T* object = get_object(_memory_indices[source]);
    if( object == nullptr )
        return;

    spawn( dest, std::forward<const T&>(*object) );
}

template<typename T, typename I>
void ObjectChunksTrait<T, I>::dispose(I index)
{
    ENSURE(index < _memory_indices.size());
    if( _memory_indices[index] == MemoryChunks<I>::invalid )
        return;

    T* object = static_cast<T*>(MemoryChunks<I>::get_element(_memory_indices[index]));
    if( object ) object->~T();
    MemoryChunks<I>::free(_memory_indices[index]);
    _memory_indices[index] = MemoryChunks<I>::invalid;
}

template<typename T, typename I>
INLINE T* ObjectChunksTrait<T, I>::get_object(I index)
{
    ENSURE(index < _memory_indices.size());
    return _memory_indices[index] == MemoryChunks<I>::invalid ?
        nullptr :
        static_cast<T*>(MemoryChunks<I>::get_element(_memory_indices[index]));
}
