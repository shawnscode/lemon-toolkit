// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF POOL
INLINE MemoryChunks::index_type MemoryChunks::size() const
{
    return _total_elements - _available;
}

INLINE MemoryChunks::index_type MemoryChunks::chunk_size() const
{
    return _chunk_size;
}

INLINE MemoryChunks::index_type MemoryChunks::element_size() const
{
    return _element_size;
}

INLINE void* MemoryChunks::get_element(index_type index)
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

INLINE const void* MemoryChunks::get_element(index_type index) const
{
    if( index >= _total_elements )
        return nullptr;

    return static_cast<void*>(_chunks[index/_chunk_size] + (index%_chunk_size)*_element_size);
}

template<typename T>
template<typename ... Args>
T* ObjectChunks<T>::spawn(Args&& ... args)
{
    auto object = MemoryChunks::malloc();
    if( object == nullptr ) return nullptr;

    ::new(object) T(std::forward<Args>(args)...);
    return object;
}

template<typename T>
void ObjectChunks<T>::dispose(T* object)
{
    if( object != nullptr )
    {
        object->~T();
        MemoryChunks::free(object);
    }
}

template<typename T, typename I, size_t DS>
IndexedObjectChunks<T, I, DS>::IndexedObjectChunks(index_type chunk_size)
: MemoryChunks(sizeof(T), chunk_size)
{
    static_assert( std::numeric_limits<I>::is_integer,
        "index type of IndexedObjectChunks should be integer." );
    _objects.resize(DS, nullptr);
}

template<typename T, typename I, size_t DS>
IndexedObjectChunks<T, I, DS>::~IndexedObjectChunks()
{
    for( auto ptr : _objects )
        if( ptr != nullptr )
            ptr->~T();
}

template<typename T, typename I, size_t DS>
template<typename ... Args> T* IndexedObjectChunks<T, I, DS>::spawn(I index, Args&& ... args)
{
    auto object = MemoryChunks::malloc();
    if( object == nullptr ) return nullptr;

    ::new(object) T(std::forward<Args>(args)...);
    _top = std::max(_top, (size_t)index+1);

    if( !_fallback )
    {
        for( auto i = 0; i < _objects.size(); i++ )
        {
            if( _objects[i] == nullptr )
            {
                _objects[i] = static_cast<T*>(object);
                _redirect_index[i] = (size_t)index;
                return static_cast<T*>(object);
            }
        }

        auto clone = std::move(_objects);
        _fallback = true;
        _objects.resize(_top, nullptr);

        for( auto i = 0; i < clone.size(); i++ )
            _objects[_redirect_index[i]] = clone[i];
        _objects[(size_t)index] = static_cast<T*>(object);
        return static_cast<T*>(object);
    }

    if( _objects.size() < _top )
        _objects.resize(_top, nullptr);

    _objects[(size_t)index] = static_cast<T*>(object);
    return static_cast<T*>(object);
}

template<typename T, typename I, size_t DS>
void IndexedObjectChunks<T, I, DS>::dispose(I index)
{
    if( !_fallback )
    {
        for( auto i = 0; i < DS; i++ )
        {
            if( _redirect_index[i] == (size_t)index )
            {
                if( _objects[i] != nullptr )
                {
                    _objects[i]->~T();
                    MemoryChunks::free(_objects[i]);
                    _objects[i] = nullptr;
                }
                return;
            }
        }

        FATAL("fix this.");
    }

    if( _objects.size() > index && _objects[index] != nullptr )
    {
        _objects[index]->~T();
        MemoryChunks::free(_objects[index]);
        _objects[index] = nullptr;
    }
}

template<typename T, typename I, size_t DS>
T* IndexedObjectChunks<T, I, DS>::find(I index)
{
    if( !_fallback )
    {
        for( auto i = 0; i < DS; i++ )
        {
            if( _redirect_index[i] == (size_t)index )
                return _objects[i];
        }
    }

    return _objects.size() > index ? _objects[index] : nullptr;
}