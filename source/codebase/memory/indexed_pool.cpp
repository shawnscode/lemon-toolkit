// @date 2016/10/11
// @author Mao Jingkai(oammix@gmail.com)

#include <codebase/memory/indexed_pool.hpp>

NS_LEMON_BEGIN

Handle IndexedMemoryPool::malloc()
{
    auto block = MemoryPool::malloc();
    if( block == nullptr )
        return Handle();

    auto handle = _handles.create();
    if( _table.size() <= handle.get_index() )
        _table.resize(handle.get_index()+1, nullptr);

    _table[handle.get_index()] = block;
    return handle;
}

void* IndexedMemoryPool::get(Handle handle)
{
    return _handles.is_valid(handle) ? _table[handle.get_index()] : nullptr;
}

void IndexedMemoryPool::free(Handle handle)
{
    if( !_handles.is_valid(handle) )
        return;

    MemoryPool::free(_table[handle.get_index()]);
    _handles.free(handle);
    _table[handle.get_index()] = nullptr;
}

void IndexedMemoryPool::free_all()
{
    MemoryPool::free_all();
    _handles.clear();
    _table.clear();
}

NS_LEMON_END