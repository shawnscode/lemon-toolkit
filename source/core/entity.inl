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

    const auto id = TypeID::value<ComponentBase, T>();
    ASSERT( !_components_mask[object._index].test(id),
        "invalid operation(duplicated component) on entity." );

    // placement new into the component pool
    auto chunks = get_chunks<T>();
    auto component = chunks->spawn(object.get_index(), std::forward<Args>(args)...);
    // set the bit mask for this component
    _components_mask[object._index].set(id);

    //
    component->on_spawn(*this, object);
    _dispatcher->emit<EvtComponentAdded<T>>(object, *component);
    return component;
}

template<typename T>
T* EntityManager::get_component(Entity object)
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    if( !is_alive(object) )
        return nullptr;
    return get_chunks<T>()->find(object.get_index());
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

    const auto id = TypeID::value<ComponentBase, T>();
    if( _components_mask[object._index].test(id) )
    {
        T* component = get_component<T>(object);
        // broadcast this to listeners
        _dispatcher->emit<EvtComponentRemoved<T>>(object, *component);
        component->on_dispose(*this, object);
        // remove the bit mask for this component
        _components_mask[object._index].reset(id);
        // call destructor
        get_chunks<T>()->dispose(object.get_index());
    }
}

template<typename T>
INLINE bool EntityManager::has_component(Entity object) const
{
    static_assert( std::is_base_of<ComponentBase, T>::value, "T is not component." );

    const auto id = TypeID::value<ComponentBase, T>();
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
    mask.set(TypeID::value<ComponentBase, T>());
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
    }
}

template<typename T>
EntityManager::object_chunks<T>* EntityManager::get_chunks()
{
    const auto id = TypeID::value<ComponentBase, T>();
    if( _components_pool.size() < (id+1) )
    {
        _components_pool.resize(id+1, nullptr);
        _components_dispose.resize(id+1, nullptr);
    }

    if( _components_pool[id] == nullptr )
    {
        auto chunks = new object_chunks<T>(T::chunk_size);
        _components_pool[id] = chunks;
        _components_dispose[id] = [chunks, this](Entity object)
        {
            auto component = chunks->find(object.get_index());
            if( component != nullptr )
            {
                _dispatcher->emit<EvtComponentRemoved<T>>(object, *component);
                component->on_dispose(*this, object);
                chunks->dispose(object.get_index());
            }
        };
    }

    return static_cast<object_chunks<T>*>(_components_pool[id]);
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