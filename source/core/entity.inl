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
template<typename T, typename ... Args>
INLINE T* Component::add_component(Args && ... args)
{
    return _world->add_component(_object, std::forward<Args>(args)...);
}

template<typename T>
INLINE T* Component::get_component()
{
    return _world->get_component<T>(_object);
}

template<typename T>
INLINE const T* Component::get_component() const
{
    return _world->get_component<T>(_object);
}

template<typename T>
INLINE void Component::remove_component()
{
    return _world->remove_component<T>(_object);
}

template<typename ... Args>
INLINE bool Component::has_components() const
{
    return _world->has_components<Args...>(_object);
}

INLINE ComponentMask Component::get_components_mask() const
{
    return _world->get_components_mask(_object);
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
    static_assert( std::is_base_of<Component, T>::value, "T is not component." );

    if( !is_alive(object) )
        return nullptr;

    const auto id = TypeInfo::id<Component, T>();
    ASSERT( !_components_mask[object._index].test(id),
        "invalid operation(duplicated component) on entity." );

    // placement new into the component pool
    auto chunks = get_chunks<T>();
    auto component = chunks->spawn(object.get_index(), std::forward<Args>(args)...);
    // set the bit mask for this component
    _components_mask[object._index].set(id);

    //
    component->_world = this;
    component->_object = object;
    component->on_spawn();

    emit<EvtComponentAdded<T>>(object, *component);
    emit<EvtEntityModified>(object);
    return component;
}

template<typename T>
T* EntityManager::get_component(Entity object)
{
    static_assert( std::is_base_of<Component, T>::value, "T is not component." );

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
    static_assert( std::is_base_of<Component, T>::value, "T is not component." );

    if( !is_alive(object) )
        return;

    const auto id = TypeInfo::id<Component, T>();
    if( _components_mask[object._index].test(id) )
    {
        T* component = get_component<T>(object);
        component->on_dispose();
        // remove the bit mask for this component
        _components_mask[object._index].reset(id);
        // broadcast this to listeners
        emit<EvtComponentRemoved<T>>(object, *component);
        emit<EvtEntityModified>(object);
        // call destructor
        get_chunks<T>()->dispose(object.get_index());
    }
}

template<typename T>
INLINE bool EntityManager::has_component(Entity object) const
{
    static_assert( std::is_base_of<Component, T>::value, "T is not component." );

    const auto id = TypeInfo::id<Component, T>();
    return is_alive(object) && _components_mask[object._index].test(id);
}

template<>
INLINE bool EntityManager::has_components(Entity object) const
{
    return true;
}

template<typename ... Args>
INLINE bool EntityManager::has_components(Entity object) const
{
    return has_components_t<Args...>();
}

template<typename T>
INLINE bool EntityManager::has_components_t(Entity object) const
{
    return has_component<T>(object);
}

template<typename T1, typename T2, typename ... Args>
INLINE bool EntityManager::has_components_t(Entity object) const
{
    return has_component<T1>(object) | has_components_t<T2, Args...>(object);
}

INLINE ComponentMask EntityManager::get_components_mask(Entity object) const
{
    if( !is_alive(object) )
        return ComponentMask();

    return _components_mask[object._index];
}

template<>
INLINE ComponentMask EntityManager::get_components_mask() const
{
    return ComponentMask();
}

template<typename ... Args>
INLINE ComponentMask EntityManager::get_components_mask() const
{
    return get_components_mask_t<Args...>();
}

template<typename T>
INLINE ComponentMask EntityManager::get_components_mask_t() const
{
    static_assert( std::is_base_of<Component, T>::value, "T is not component." );

    ComponentMask mask;
    mask.set(TypeInfo::id<Component, T>());
    return mask;
}

template<typename T1, typename T2, typename ... Args>
INLINE ComponentMask EntityManager::get_components_mask_t() const
{
    return get_components_mask_t<T1>() | get_components_mask_t<T2, Args ...>();
}

INLINE EntityManager::view<> EntityManager::find_entities()
{
    return EntityManager::view<>(*this);
}

INLINE EntityManager::const_view<> EntityManager::find_entities() const
{
    return EntityManager::const_view<>(*this);
}

template<typename ... T>
INLINE EntityManager::view<T...> EntityManager::find_entities_with()
{
    return EntityManager::view<T...>(*this);
}

template<typename ... T>
INLINE EntityManager::const_view<T...> EntityManager::find_entities_with() const
{
    return EntityManager::const_view<T...>(*this);
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
    const auto id = TypeInfo::id<Component, T>();
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
                emit<EvtComponentRemoved<T>>(object, *component);
                component->on_dispose();
                chunks->dispose(object.get_index());
            }
        };
    }

    return static_cast<object_chunks<T>*>(_components_pool[id]);
}

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
template<typename T>
EntityManager::iterator_t<T>::iterator_t(T& manager, Entity::index_type index, ComponentMask mask)
: _world(manager), _index(index), _mask(mask)
{
    if( !is_match(_index) )
        find_next_available();
}

template<typename T>
INLINE void EntityManager::iterator_t<T>::find_next_available()
{

    while( _index < _world._versions.size() )
    {
        _index ++;
        if( is_match(_index) ) break;
    }
}

template<typename T>
INLINE bool EntityManager::iterator_t<T>::is_match(Entity::index_type index)
{
    return
        index < _world._versions.size() &&
        (_world._versions[index] & 0x1) != 0 &&
        (_world._components_mask[index] & _mask) == _mask;
}

template<typename T>
INLINE EntityManager::iterator_t<T>& EntityManager::iterator_t<T>::operator ++ ()
{
    find_next_available();
    return *this;
}

template<typename T>
INLINE EntityManager::iterator_t<T> EntityManager::iterator_t<T>::operator ++ (int dummy)
{
    auto tmp = *this;
    find_next_available();
    return tmp;
}

template<typename T>
INLINE bool EntityManager::iterator_t<T>::operator == (const EntityManager::iterator_t<T>& rhs) const
{
    return
        (&_world == &rhs._world) &&
        ((_index >= _world._versions.size())  == (rhs._index >= _world._versions.size()));
}

template<typename T>
INLINE bool EntityManager::iterator_t<T>::operator != (const EntityManager::iterator_t<T>& rhs) const
{
    return !(*this == rhs);
}

template<typename T>
INLINE Entity EntityManager::iterator_t<T>::operator * () const
{
    return Entity(_index, _world._versions[_index]);
}

template<typename T, typename ... Args>
EntityManager::view_t<T, Args...>::view_t(T& manager)
: _world(manager), _mask(manager.template get_components_mask<Args...>())
{}

template<typename T, typename ... Args>
INLINE EntityManager::iterator_t<T> EntityManager::view_t<T, Args...>::begin() const
{
    return EntityManager::iterator_t<T>(_world, 0, _mask);
}

template<typename T, typename ... Args>
INLINE EntityManager::iterator_t<T> EntityManager::view_t<T, Args...>::end() const
{
    return EntityManager::iterator_t<T>(_world, Entity::invalid, _mask);
}

template<typename T, typename ... Args>
INLINE void EntityManager::view_t<T, Args...>::visit(const visitor& cb)
{
    for( auto cursor : *this )
        cb(cursor, *view_t<T, Args...>::_world.template get_component<Args>(cursor) ...);
}