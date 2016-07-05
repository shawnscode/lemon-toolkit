// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF ENTITY
INLINE bool Entity::operator == (const Entity& rh) const
{
    return _index == rh._index && _version == rh._version;
}

INLINE bool Entity::operator != (const Entity& rh) const
{
    return !(*this == rh);
}

INLINE bool Entity::operator < (const Entity& rh) const
{
    return _version == rh._version ? _index < rh._index : _version < rh._version;
}

INLINE Entity::index_type Entity::get_index() const
{
    return _index;
}

INLINE Entity::index_type Entity::get_version() const
{
    return _version;
}

INLINE void Entity::invalidate()
{
    _index = invalid;
    _version = invalid;
}

// INCLUDED METHODS OF COMPONENT
template<typename T>
ComponentChunksTrait<T>::~ComponentChunksTrait()
{
    for( Entity::index_type index : _memory_indices )
    {
        T* component = static_cast<T*>(chunks_type::get_element(index));
        if( component ) component->~T();
    }
}

template<typename T>
template<typename ... Args>
void* ComponentChunksTrait<T>::spawn(EntityManager& world, Entity object, Args && ... args)
{
    ENSURE( _memory_indices[object.get_index()] == chunks_type::invalid );

    Entity::index_type index = chunks_type::malloc();
    if( index == chunks_type::invalid )
        return nullptr;

    void* component = chunks_type::get_element(index);
    ::new(component) T(std::forward<Args>(args)...);

    _memory_indices[object.get_index()] = index;
    when_spawn(object, *static_cast<T*>(component));
    return component;
}

template<typename T>
void ComponentChunksTrait<T>::dispose(EntityManager& world, Entity object)
{
    ENSURE( object.get_index() < _memory_indices.size() );

    Entity::index_type index = _memory_indices[object.get_index()];
    if( index == chunks_type::invalid )
        return;

    T* component = static_cast<T*>(chunks_type::get_element(index));
    when_dispose(object, *component);

    component->~T();
    chunks_type::free(index);
    _memory_indices[object.get_index()] = chunks_type::invalid;
}

template<typename T>
INLINE T* ComponentChunksTrait<T>::get(Entity object)
{
    ENSURE( object.get_index() < _memory_indices.size() );

    Entity::index_type index = _memory_indices[object.get_index()];
    if( index == chunks_type::invalid )
        return nullptr;
    return static_cast<T*>(chunks_type::get_element(index));
}

template<typename T>
INLINE void ComponentChunksTrait<T>::resize(Entity::index_type size)
{
    _memory_indices.resize(size, chunks_type::invalid);
}

// INCLUDED METHODS OF ENTITY MANAGER
INLINE size_t EntityManager::size() const
{
    return _versions.size() - _freeslots.size();
}

INLINE size_t EntityManager::capacity() const
{
    return _versions.size();
}

INLINE bool EntityManager::is_alive(Entity object) const
{
    return
        object._index < _versions.size() &&
        object._version == _versions[object._index];
}

template<typename T, typename ... Args>
Entity EntityManager::spawn_with(Args&& ... args)
{
    Entity object = spawn();
    add_component<T>(object, std::forward<Args>(args)...);
    return object;
}

template<typename T, typename ... Args>
T* EntityManager::add_component(Entity object, Args&& ... args)
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    if( !is_alive(object) )
        return nullptr;

    const auto id = ComponentTraitInfo<T>::id();
    ASSERT( !_components_mask[object._index].test(id),
        "invalid operation(duplicated component) on entity." );

    // placement new into the component pool
    auto chunks = get_chunks<T>();
    auto component = static_cast<T*>(chunks->spawn(*this, object, std::forward<Args>(args)...));
    // set the bit mask for this component
    _components_mask[object._index].set(id);
    _dispatcher->emit<EvtComponentAdded<T>>(object, *component);
    return component;
}

template<typename T>
T* EntityManager::get_component(Entity object)
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    if( !is_alive(object) )
        return nullptr;
    return get_chunks<T>()->get(object);
}

template<typename ... T>
INLINE std::tuple<T*...> EntityManager::get_components(Entity object)
{
    return std::make_tuple(get_component<T>(object)...);
}

template<typename T>
void EntityManager::remove_component(Entity object)
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    if( !is_alive(object) )
        return;

    const auto id = ComponentTraitInfo<T>::id();
    if( _components_mask[object._index].test(id) )
    {
        T* component = get_component<T>(object);
        // broadcast this to listeners
        _dispatcher->emit<EvtComponentRemoved<T>>(object, *component);
        // remove the bit mask for this component
        _components_mask[object._index].reset(id);
        // call destructor
        get_chunks<T>()->dispose(object._index);
    }
}

template<typename T>
INLINE bool EntityManager::has_component(Entity object) const
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    const auto id = ComponentTraitInfo<T>::id();
    return is_alive(object) && _components_mask[object._index].test(id);
}

INLINE ComponentMask EntityManager::get_components_mask(Entity object) const
{
    if( !is_alive(object) )
        return ComponentMask();

    return _components_mask[object._index];
}

template<typename T>
INLINE ComponentMask EntityManager::get_components_mask() const
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    ComponentMask mask;
    mask.set(ComponentTraitInfo<T>::id());
    return mask;
}

template<typename T1, typename T2, typename ... Args>
INLINE ComponentMask EntityManager::get_components_mask() const
{
    return get_components_mask<T1>() | get_components_mask<T2, Args ...>();
}

template<typename ... T>
INLINE EntityManager::view_trait<T...> EntityManager::find_entities_with()
{
    return EntityManager::view_trait<T...>(*this);
}

INLINE EntityManager::view EntityManager::find_entities()
{
    return EntityManager::view(*this, ComponentMask());
}

INLINE EventManager& EntityManager::get_dispatcher()
{
    return *_dispatcher;
}

INLINE void EntityManager::accomodate_entity(uint32_t index)
{
    if( _components_mask.size() < (index+1) )
    {
        _components_mask.resize(index+1);
        _versions.resize(index+1);
        for( auto p : _components_pool)
            if( p != nullptr )
                p->resize(index+1);
    }
}

template<typename T>
EntityManager::object_chunks_trait<T>* EntityManager::get_chunks()
{
    const auto id = ComponentTraitInfo<T>::id();
    if( _components_pool.size() < (id+1) )
        _components_pool.resize((id+1), nullptr);

    if( _components_pool[id] == nullptr )
    {
        auto chunks = new object_chunks_trait<T>(T::chunk_size);
        chunks->resize(_components_mask.size());
        chunks->when_spawn = [&](Entity o, T& c)
        {
            c.on_spawn(*this, o);
        };
        chunks->when_dispose = [&](Entity o, T& c)
        {
            c.on_dispose(*this, o);
        };
        _components_pool[id] = chunks;
    }

    return static_cast<EntityManager::object_chunks_trait<T>*>(_components_pool[id]);
}

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
INLINE void EntityManager::iterator::find_next_available()
{
    while(
        _index < _world._versions.size() &&
        ((_world._versions[_index] & 0x1) == 0 ||
         (_world._components_mask[_index] & _mask) != _mask))
    {
        _index ++;
    }

    if( _index == _world._versions.size() )
        _index = Entity::invalid;
}

INLINE EntityManager::iterator& EntityManager::iterator::operator ++ ()
{
    ENSURE( _index != Entity::invalid );

    _index ++;
    find_next_available();
    return *this;
}

INLINE bool EntityManager::iterator::operator == (const EntityManager::iterator& rh) const
{
    return &_world == &rh._world && _index == rh._index;
}

INLINE bool EntityManager::iterator::operator != (const EntityManager::iterator& rh) const
{
    return &_world != &rh._world || _index != rh._index;
}

INLINE Entity EntityManager::iterator::operator * () const
{
    return Entity(_index, _world._versions[_index]);
}

INLINE EntityManager::iterator EntityManager::view::begin() const
{
    return EntityManager::iterator(_world, 0, _mask);
}

INLINE EntityManager::iterator EntityManager::view::end() const
{
    return EntityManager::iterator(_world, Entity::invalid, _mask);
}

template<typename ...T>
INLINE void EntityManager::view_trait<T...>::visit(const visitor& cb)
{
    for( auto cursor : *this )
        cb(cursor, *_world.get_component<T>(cursor) ...);
}