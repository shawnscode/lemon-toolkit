// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/private/world.hpp>

NS_LEMON_CORE_BEGIN

IndexedMemoryChunks::IndexedMemoryChunks(World& world, size_t size, size_t chunk_size, const internal::destructor& cb)
: _world(world), _destructor(cb), FixedSizeAllocator(size, chunk_size)
{
    _objects.resize(kFallbackComponentSize, nullptr);
}

IndexedMemoryChunks::~IndexedMemoryChunks()
{
    for( Handle::index_t i = 0; i < _objects.size(); i++ )
    {
        if( _objects[i] != nullptr )
            _destructor(_world.get(i), _objects[i]);
    }
}

void* IndexedMemoryChunks::malloc_with_index(Handle::index_t index)
{
    auto object = FixedSizeAllocator::malloc();
    if( object == nullptr )
        return nullptr;

    _top = std::max(_top, (unsigned)index+1);

    if( !_fallback )
    {
        for( unsigned i = 0; i < _objects.size(); i++ )
        {
            if( _objects[i] == nullptr )
            {
                _objects[i] = object;
                _redirect[i] = (unsigned)index;
                return object;
            }
        }

        auto clone = std::move(_objects);
        _fallback = true;
        _objects.resize(_top, nullptr);

        for( unsigned i = 0; i < clone.size(); i++ )
            _objects[_redirect[i]] = clone[i];
        _objects[(unsigned)index] = object;
        return object;
    }

    if( _objects.size() < _top )
        _objects.resize(_top, nullptr);

    _objects[(unsigned)index] = object;
    return object;
}

void IndexedMemoryChunks::free_with_index(Handle::index_t index)
{
    if( !_fallback )
    {
        for( unsigned i = 0; i < kFallbackComponentSize; i++ )
        {
            if( _redirect[i] == (unsigned)index )
            {
                if( _objects[i] != nullptr )
                {
                    _destructor(_world.get(index), _objects[i]);
                    FixedSizeAllocator::free(_objects[i]);
                    _objects[i] = nullptr;
                }
                return;
            }
        }
        return;
    }

    if( _objects.size() > index && _objects[index] != nullptr )
    {
        _destructor(_world.get(index), _objects[index]);
        FixedSizeAllocator::free(_objects[index]);
        _objects[index] = nullptr;
    }
}

void* IndexedMemoryChunks::get(Handle::index_t index)
{
    if( !_fallback )
    {
        for( unsigned i = 0; i < kFallbackComponentSize; i++ )
        {
            if( _redirect[i] == (unsigned)index )
                return _objects[i];
        }
        return nullptr;
    }

    return _objects.size() > index ? _objects[index] : nullptr;
}

bool World::initialize()
{
    return true;
}

void World::dispose()
{
    for( Handle::index_t i = 0; i < _components.size(); i++ )
    {
        if( _components[i] != nullptr )
        {
            delete _components[i];
            _components[i] = nullptr;
        }
    }

    for( unsigned i = 0; i < _masks.size(); i++ )
    {
        _masks[i].reset();
    }

    _handles.clear();
}

Handle World::spawn()
{
    auto handle = _handles.create();

    if( _masks.size() < (handle.get_index()+1) )
        _masks.resize(handle.get_index()+1);

    return handle;
}

void World::recycle(Handle object)
{
    if( !alive(object) )
        return;

    const auto mask = _masks[object.get_index()];
    for( unsigned i = 0; i < _components.size(); i++ )
    {
        auto chunks = _components[i];
        if( chunks != nullptr && mask.test(i) )
            chunks->free_with_index(object.get_index());
    }

    _masks[object.get_index()].reset();
    _handles.free(object);
}

bool World::register_component(TypeInfo::index_t id, size_t size, size_t chunk_size, const internal::destructor& destructor)
{
    if( _components.size() < (id+1) )
        _components.resize(id+1);

    if( _components[id] == nullptr )
    {
        _components[id] = new (std::nothrow) IndexedMemoryChunks(*this, size, chunk_size, destructor);
        if( _components[id] == nullptr )
        {
            LOGW("failed to register component due to out of memory.");
            return false;
        }
    }
    return true;
}

void* World::add_component(TypeInfo::index_t id, Handle object)
{
    if( !alive(object) )
        return nullptr;

    ASSERT( _components.size() > id && _components[id] != nullptr,
        "can't add component without register it first." );

    auto chunk = _components[id]->malloc_with_index(object.get_index());
    _masks[object.get_index()].set(id);
    return chunk;
}

void* World::get_component(TypeInfo::index_t id, Handle object)
{
    if( !alive(object) )
        return nullptr;

    if( _components.size() <= id || _components[id] == nullptr )
        return nullptr;

    return _components[id]->get(object.get_index());
}

void World::remove_component(TypeInfo::index_t id, Handle object)
{
    if( !alive(object) )
        return;

    if( _masks[object.get_index()].test(id) )
    {
        _masks[object.get_index()].reset(id);
        _components[id]->free_with_index(object.get_index());
    }
}

Handle World::find_first_available(ComponentMask mask)
{
    for( auto handle : _handles )
    {
        if( (_masks[handle.get_index()] & mask) == mask )
            return handle;
    }

    return Handle();
}

Handle World::find_next_available(Handle handle, ComponentMask mask)
{
    auto iterator = ++ReuseableHandleSet::iterator(_handles, handle);
    for( ; iterator != _handles.end(); iterator++ )
    {
        if( (_masks[(*iterator).get_index()] & mask) == mask )
            return *iterator;
    }

    return Handle();
}

NS_LEMON_CORE_END