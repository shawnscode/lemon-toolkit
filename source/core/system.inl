// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

template<typename C>
void SystemWithEntities<C>::on_spawn(SystemManager& manager)
{
    manager.get_dispatcher().template subscribe<EvtComponentAdded<C>>(*this);
    manager.get_dispatcher().template subscribe<EvtComponentRemoved<C>>(*this);
}

template<typename C>
void SystemWithEntities<C>::on_dispose(SystemManager& manager)
{
    manager.get_dispatcher().template unsubscribe<EvtComponentAdded<C>>(*this);
    manager.get_dispatcher().template unsubscribe<EvtComponentRemoved<C>>(*this);
}

template<typename C>
void SystemWithEntities<C>::receive(const EvtComponentAdded<C>& evt)
{
    _entities[evt.entity] = &evt.component;
}

template<typename C>
void SystemWithEntities<C>::receive(const EvtComponentRemoved<C>& evt)
{
    _entities.erase(evt.entity);
}

// INCLUDED METHODS OF SYSTEM MANAGER
INLINE EntityManager& SystemManager::get_world()
{
    return _world;
}

INLINE EventManager& SystemManager::get_dispatcher()
{
    return _world.get_dispatcher();
}

template<typename S, typename ... Args>
S* SystemManager::ensure(Args && ... args)
{
    auto found = _systems.find(S::type());
    if( found == _systems.end() )
        return add(std::forward<Args>(args)...);

    return found->second;
}

template<typename S, typename ... Args>
S* SystemManager::add(Args && ... args)
{
    auto found = _systems.find(TypeID::value<System, S>());
    ASSERT( found == _systems.end(), "[ECS] duplicated system.");

    auto sys = new (std::nothrow) S(_world, std::forward<Args>(args) ...);
    sys->on_spawn(*this);
    _systems.insert(std::make_pair(TypeID::value<System, S>(), sys));
    return sys;
}

template<typename S>
void SystemManager::remove()
{
    auto found = _systems.find(TypeID::value<System, S>());
    if( found != _systems.end() )
    {
        found->on_dispose(*this);
        delete found->second;
        _systems.erase(found);
    }
}

template<typename S>
S* SystemManager::get()
{
    auto found = _systems.find(TypeID::value<System, S>());
    if( found != _systems.end() )
        return static_cast<S*>(found->second);
    return nullptr;
}