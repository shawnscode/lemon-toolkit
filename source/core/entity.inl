// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF ENTITY
INLINE bool Entity::is_alive() const
{
    return _world.is_alive(*this);
}

INLINE Entity::operator bool () const
{
    return is_alive();
}

INLINE bool Entity::operator == (const Entity& rh) const
{
    return &_world == &rh._world && _index == rh._index && _version == rh._version;
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

template<typename T, typename ... Args>
INLINE T* Entity::add_component(Args && ... args)
{
    return _world.add_component<T>(*this, std::forward<Args>(args)...);
}

template<typename T>
INLINE T* Entity::add_component(const T& rh)
{
    return _world.add_component<T>(*this, std::forward<const T&>(rh));
}

template<typename T>
INLINE T* Entity::get_component()
{
    return _world.get_component<T>(*this);
}

template<typename ... T>
INLINE std::tuple<T*...> Entity::get_components()
{
    return _world.get_components<T...>(*this);
}

template<typename T>
INLINE bool Entity::has_component() const
{
    return _world.has_component<T>(*this);
}

template<typename T>
INLINE void Entity::remove_component()
{
    _world.remove_component<T>(*this);
}

INLINE ComponentMask Entity::get_components_mask() const
{
    return _world.get_components_mask(*this);
}

INLINE void Entity::invalidate()
{
    _index = invalid;
    _version = invalid;
}

INLINE void Entity::dispose()
{
    _world.dispose(*this);
    invalidate();
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
T* EntityManager::add_component(Entity object, Args&& ... args)
{
    ASSERT( is_alive(object),
        "invalid operation(add_component) on a nonexistent entity.");

    const auto id = ComponentTraitInfo<T>::id();
    ASSERT( !_components_mask[object._index].test(id),
        "invalid operation(duplicated component) on entity." );

    // placement new into the component pool
    T* component = get_chunks<T>()->spawn(object._index, std::forward<Args>(args) ...);
    // set the bit mask for this component
    _components_mask[object._index].set(id);

    _dispatcher.emit<EvtComponentAdded<T>>(object, *component);
    return component;
}

template<typename T>
T* EntityManager::get_component(Entity object)
{
    ASSERT( is_alive(object),
        "invalid operation(get_component) on a nonexistent entity.");

    const auto id = ComponentTraitInfo<T>::id();
    if( id >= _components_pool.size() )
        return nullptr;

    return static_cast<object_chunks_trait<T>*>(_components_pool[id])->get_object(object._index);
}

template<typename ... T>
INLINE std::tuple<T*...> EntityManager::get_components(Entity object)
{
    return std::make_tuple(get_component<T>(object)...);
}

template<typename T>
void EntityManager::remove_component(Entity object)
{
    ASSERT( is_alive(object),
        "invalid operation(add_component) on a nonexistent entity.");

    const auto id = ComponentTraitInfo<T>::id();
    if( _components_mask[object._index].test(id) )
    {
        T* component = get_component<T>(object);
        _dispatcher.emit<EvtComponentRemoved<T>>(object, *component);
        // remove the bit mask for this component
        _components_mask[object._index].reset(id);
        // call destructor
        get_chunks<T>()->dispose(object._index);
    }
}

template<typename T>
INLINE bool EntityManager::has_component(Entity object) const
{
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
        auto chunks = new object_chunks_trait<T>(kEntPoolChunkSize);
        chunks->resize(_components_mask.size());
        _components_pool[id] = chunks;
    }

    return static_cast<EntityManager::object_chunks_trait<T>*>(_components_pool[id]);
}

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
INLINE void EntityManager::Iterator::find_next_available()
{
    while(
        _index < _manager._versions.size() &&
        ((_manager._versions[_index] & 0x1) == 0 ||
         (_manager._components_mask[_index] & _mask) != _mask))
    {
        _index ++;
    }

    if( _index == _manager._versions.size() )
        _index = Entity::invalid;
}

INLINE EntityManager::Iterator& EntityManager::Iterator::operator ++ ()
{
    ENSURE( _index != Entity::invalid );

    _index ++;
    find_next_available();
    return *this;
}

INLINE bool EntityManager::Iterator::operator == (const EntityManager::Iterator& rh) const
{
    return &_manager == &rh._manager && _index == rh._index;
}

INLINE bool EntityManager::Iterator::operator != (const EntityManager::Iterator& rh) const
{
    return &_manager != &rh._manager || _index != rh._index;
}

INLINE Entity EntityManager::Iterator::operator * () const
{
    return Entity(_manager, _index, _manager._versions[_index]);
}

INLINE EntityManager::iterator EntityManager::View::begin() const
{
    return EntityManager::iterator(_manager, 0, _mask);
}

INLINE EntityManager::iterator EntityManager::View::end() const
{
    return EntityManager::iterator(_manager, Entity::invalid, _mask);
}

template<typename ...T>
INLINE void EntityManager::ViewTrait<T...>::each(callback cb)
{
    for( auto cursor : *this )
        cb(cursor, *(cursor.template get_component<T>()) ...);
}