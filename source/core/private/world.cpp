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
    for( Entity::index_type i = 0; i < _objects.size(); i++ )
    {
        if( _objects[i] != nullptr )
            _destructor(_world.get(i), _objects[i]);
    }
}

void* IndexedMemoryChunks::malloc_with_index(Entity::index_type index)
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

void IndexedMemoryChunks::free_with_index(Entity::index_type index)
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

void* IndexedMemoryChunks::get(Entity::index_type index)
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
    for( Entity::index_type i = 0; i < _components.size(); i++ )
    {
        if( _components[i] != nullptr )
        {
            delete _components[i];
            _components[i] = nullptr;
        }
    }

    for( unsigned i = 0; i < _versions.size(); i++ )
    {
        if( (_versions[i] & 0x1) == 1 )
            _versions[i] ++;
    }

    for( unsigned i = 0; i < _masks.size(); i++ )
    {
        _masks[i].reset();
    }
}

Entity World::spawn()
{
    Entity::index_type index, version;
    if( _freeslots.empty() )
    {
        index = _incremental_index++;
        if( _masks.size() < _incremental_index )
        {
            _masks.resize(_incremental_index);
            _versions.resize(_incremental_index);
        }

        ASSERT( _incremental_index != Entity::invalid,
            "too much entities,"
            "please considering change the representation of Entity::index_type." );
        _versions[index] = 1;
    }
    else
    {
        index = _freeslots.back();
        _freeslots.pop_back();
        _versions[index] ++;
    }

    version = _versions[index];
    return Entity(index, version);
}

void World::recycle(Entity object)
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
    _versions[object.get_index()] ++;
    _freeslots.push_back(object.get_index());

    ASSERT( _versions[object.get_index()] != Entity::invalid,
        "too much reusages of this entity,"
        "please considering change the representation of Entity::index_type." );
}

bool World::register_component(TypeInfo::index_type id, size_t size, size_t chunk_size, const internal::destructor& destructor)
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

void* World::add_component(TypeInfo::index_type id, Entity object)
{
    if( !alive(object) )
        return nullptr;

    ASSERT( _components.size() > id && _components[id] != nullptr,
        "can't add component without register it first." );

    auto chunk = _components[id]->malloc_with_index(object.get_index());
    _masks[object.get_index()].set(id);
    return chunk;
}

void* World::get_component(TypeInfo::index_type id, Entity object)
{
    if( !alive(object) )
        return nullptr;

    if( _components.size() <= id || _components[id] == nullptr )
        return nullptr;

    return _components[id]->get(object.get_index());
}

void World::remove_component(TypeInfo::index_type id, Entity object)
{
    if( !alive(object) )
        return;

    if( _masks[object.get_index()].test(id) )
    {
        _masks[object.get_index()].reset(id);
        _components[id]->free_with_index(object.get_index());
    }
}

Entity World::find_next_available(Entity::index_type index, ComponentMask mask, bool self)
{
    if( index == Entity::invalid )
        return Entity();

    for( Entity::index_type i = self ? index : index + 1; i < Entity::invalid; i++ )
    {
        if( i < _versions.size() && (_versions[i] & 0x1) == 1 )
        {
            if( (_masks[i] & mask) == mask )
                return Entity(i, _versions[i]);
        }
    }

    return Entity();
}

NS_LEMON_CORE_END