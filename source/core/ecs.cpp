// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/ecs.hpp>
#include <codebase/memory/allocator.hpp>

NS_LEMON_CORE_BEGIN

struct ECSWorld
{
    struct IndexedMemoryChunks : public FixedSizeAllocator
    {
        IndexedMemoryChunks(ECSWorld& world, size_t size, size_t chunk_size, const ecs::destructor& cb)
        : _world(world), _destructor(cb), FixedSizeAllocator(size, chunk_size)
        {
            _objects.resize(8, nullptr);
        }

        ~IndexedMemoryChunks()
        {
            for( Entity::index_type i = 0; i < _objects.size(); i++ )
            {
                if( _objects[i] != nullptr )
                    _destructor(_world.get(i), _objects[i]);
            }
        }

        void* malloc_with_index(Entity::index_type index)
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

        void free_with_index(Entity::index_type index)
        {
            if( !_fallback )
            {
                for( unsigned i = 0; i < 8; i++ )
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

        void* get(Entity::index_type index)
        {
            if( !_fallback )
            {
                for( unsigned i = 0; i < 8; i++ )
                {
                    if( _redirect[i] == (unsigned)index )
                        return _objects[i];
                }
                return nullptr;
            }

            return _objects.size() > index ? _objects[index] : nullptr;
        }

    protected:
        ECSWorld&           _world;
        bool                _fallback = false;
        ecs::destructor     _destructor;
        unsigned            _redirect[8];
        unsigned            _top = 8;
        std::vector<void*>  _objects;
    };

    ~ECSWorld()
    {
        reset();
    }

    void reset()
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

    Entity spawn()
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

    void recycle(Entity object)
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

    INLINE bool alive(Entity object)
    {
        return
            object.get_index() < _versions.size() &&
            object.get_version() == _versions[object.get_index()];
    }

    INLINE Entity get(Entity::index_type index)
    {
        return index < _versions.size() && (_versions[index] & 0x1) == 1 ?
            Entity(index, _versions[index]) : Entity();
    }

    bool register_component(TypeInfo::index_type id, size_t size, size_t chunk_size, const ecs::destructor& destructor)
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

    bool has_component_registered(TypeInfo::index_type id)
    {
        return _components.size() > id && _components[id] != nullptr;
    }

    void* add_component(TypeInfo::index_type id, Entity object)
    {
        if( !alive(object) )
            return nullptr;

        ASSERT( _components.size() > id && _components[id] != nullptr,
            "can't add component without register it first." );

        auto chunk = _components[id]->malloc_with_index(object.get_index());
        _masks[object.get_index()].set(id);
        return chunk;
    }

    void* get_component(TypeInfo::index_type id, Entity object)
    {
        if( !alive(object) )
            return nullptr;

        if( _components.size() <= id || _components[id] == nullptr )
            return nullptr;

        return _components[id]->get(object.get_index());
    }

    void remove_component(TypeInfo::index_type id, Entity object)
    {
        if( !alive(object) )
            return;

        if( _masks[object.get_index()].test(id) )
        {
            _masks[object.get_index()].reset(id);
            _components[id]->free_with_index(object.get_index());
        }
    }

    INLINE bool has_component(TypeInfo::index_type id, Entity object)
    {
        return alive(object) && _masks[object.get_index()].test(id);
    }

    INLINE ComponentMask get_components_mask(Entity object)
    {
        return alive(object) ? _masks[object.get_index()] : ComponentMask();
    }

    INLINE size_t size() const
    {
        return _incremental_index - _freeslots.size();
    }

    Entity find_next_available(Entity::index_type index, ComponentMask mask, bool self)
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

    INLINE size_t size(TypeInfo::index_type id)
    {
        return (_components.size() > id && _components[id] != nullptr) ?
            _components[id]->size() : 0;
    }

    INLINE size_t capacity(TypeInfo::index_type id)
    {
        return (_components.size() > id && _components[id] != nullptr) ?
            _components[id]->capacity() : 0;   
    }

protected:
    // incremented entity index for brand new and free slot
    Entity::index_type _incremental_index = 0;

    // each element in componets_pool corresponds to a Pool for a Component
    // the index into the vector is the Component::type();
    std::vector<IndexedMemoryChunks*> _components;

    // bitmask of components associated with each entity
    // the index into the vector is the Entity::Uid
    std::vector<ComponentMask> _masks;

    // entity version numbers. incremented each time an entity is destroyed
    std::vector<Entity::index_type> _versions;

    // list of available entity slots
    std::vector<Entity::index_type> _freeslots;
};

static ECSWorld* s_world = nullptr;

Entity spawn()
{
    return s_world->spawn();
}

void recycle(Entity object)
{
    s_world->recycle(object);
}

bool alive(Entity object)
{
    return s_world->alive(object);
}

ComponentMask get_components_mask(Entity object)
{
    return s_world->get_components_mask(object);
}

namespace ecs
{
    bool initialize()
    {
        ASSERT( s_world == nullptr,
            "duplicated initialization of entity component system." );

        s_world = new (std::nothrow) ECSWorld();
        return s_world != nullptr;
    }

    void dispose()
    {
        if( s_world != nullptr )
        {
            delete s_world;
            s_world = nullptr;
        }
    }

    size_t size()
    {
        return s_world->size();
    }

    void reset()
    {
        s_world->reset();
    }

    bool has_component_registered(TypeInfo::index_type id)
    {
        return s_world->has_component_registered(id);
    }

    bool register_component(TypeInfo::index_type id, size_t size, size_t chunk_size, const destructor& cb)
    {
        return s_world->register_component(id, size, chunk_size, cb);
    }

    void* add_component(TypeInfo::index_type id, Entity object)
    {
        return s_world->add_component(id, object);
    }

    void* get_component(TypeInfo::index_type id, Entity object)
    {
        return s_world->get_component(id, object);
    }

    void remove_component(TypeInfo::index_type id, Entity object)
    {
        s_world->remove_component(id, object);
    }

    bool has_component(TypeInfo::index_type id, Entity object)
    {
        return s_world->has_component(id, object);
    }

    Entity::index_type find_next_available(Entity::index_type index, ComponentMask mask, bool self)
    {
        return s_world->find_next_available(index, mask, self).get_index();
    }

    Entity get(Entity::index_type index)
    {
        return s_world->get(index);
    }

    namespace test_mem
    {
        size_t size(TypeInfo::index_type id)
        {
            return s_world->size(id);
        }

        size_t capacity(TypeInfo::index_type id)
        {
            return s_world->capacity(id);
        }
    }
}

NS_LEMON_CORE_END