// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF SYSTEM MANAGER
INLINE EntityManager& System::world()
{
    return *_world;
}

INLINE EventManager& System::dispatcher()
{
    return *_dispatcher;
}

INLINE SystemManager& System::systems()
{
    return *_systems;
}

template<typename T>
System::Type SystemTrait<T>::type()
{
    static Type cls = s_type_counter ++;
    return cls;
}

template<typename C, typename ... Args>
void RequireComponents<C, Args...>::on_attach()
{
    System::dispatcher().template subscribe<EvtComponentAdded<C>>(*this);
}

template<typename C, typename ... Args>
void RequireComponents<C, Args...>::on_detach()
{
    System::dispatcher().template unsubscribe<EvtComponentAdded<C> >(*this);
}

template<typename C, typename ... Args>
void RequireComponents<C, Args...>::receive(const EvtComponentAdded<C>& evt)
{
    add_component<Args...>(evt.entity);
}

template<typename C, typename ... Args>
template<typename D>
void RequireComponents<C, Args...>::add_component(Entity ent)
{
    if( !System::_world->template has_component<D>(ent) )
        System::_world->template add_component<D>(ent);
}

template<typename C, typename ... Args>
template<typename D, typename D1, typename ... Tails>
void RequireComponents<C, Args...>::add_component(Entity ent)
{
    add_component<D>(ent);
    add_component<D1, Tails...>(ent);
}

INLINE EventManager& SystemManager::dispatcher()
{
    return _dispatcher;
}

INLINE EntityManager& SystemManager::world()
{
    return _world;
}

template<typename S, typename ... Args>
S* SystemManager::ensure(Args && ... args)
{
    auto found = _systems.find(S::type());
    if( found == _systems.end() )
    {
        auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
        _systems.insert(std::make_pair(S::type(), sys));
        return sys;
    }

    return found->second;
}


template<typename S, typename ... Args>
S* SystemManager::add(Args && ... args)
{
    auto found = _systems.find(S::type());
    ASSERT( found == _systems.end(), "[ECS] duplicated system.");

    auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
    sys->_world = &_world;
    sys->_dispatcher = &_dispatcher;
    sys->_systems = this;

    _systems.insert(std::make_pair(S::type(), sys));
    return sys;
}

template<typename S>
void SystemManager::remove()
{
    auto found = _systems.find(S::type());
    if( found != _systems.end() )
    {
        delete found->second;
        _systems.erase(found);
    }
}

template<typename S>
S* SystemManager::get()
{
    auto found = _systems.find(S::type());
    if( found != _systems.end() )
        return static_cast<S*>(found->second);
    return nullptr;
}