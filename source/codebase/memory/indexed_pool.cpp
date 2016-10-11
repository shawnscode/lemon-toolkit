// @date 2016/10/11
// @author Mao Jingkai(oammix@gmail.com)

#include <codebase/memory/indexed_pool.hpp>

NS_LEMON_BEGIN

IndexedMemoryPool::IndexedMemoryPool(size_t element_size, size_t chunk_size)
: _allocator(element_size, chunk_size)
{}

Handle IndexedMemoryPool::malloc()
{
    auto block = _allocator.malloc();
    if( block == nullptr )
        return Handle();

    auto handle = _handles.create();
    if( _table.size() <= handle.get_index() )
        _table.resize(handle.get_index(), nullptr);

    _table[handle.get_index()] = block;
    return handle;
}

void IndexedMemoryPool::free(Handle handle)
{
    if( !_handles.is_valid(handle) )
        return;

    _allocator.free(_table[handle.get_index()]);
    _handles.free(handle);
    _table[handle.get_index()] = nullptr;
}

IndexedObjectPool::IndexedObjectPool(
    size_t element_size,
    size_t chunk_size,
    const callback& ctor,
    const callback& dtor)
: IndexedMemoryPool(element_size, chunk_size), _constructor(ctor), _destructor(dtor)
{}

IndexedObjectPool::~IndexedObjectPool()
{
    for( auto handle : _handles )
        free(handle);
}

Handle IndexedObjectPool::malloc()
{
    auto handle = IndexedMemoryPool::malloc();
    auto object = get(handle);
    if( object != nullptr )
        _constructor(object);
    return handle;
}

void IndexedObjectPool::free(Handle handle)
{
    auto object = get(handle);
    if( object == nullptr )
        return;

    _destructor(object);
    IndexedMemoryPool::free(handle);
}

NS_LEMON_END