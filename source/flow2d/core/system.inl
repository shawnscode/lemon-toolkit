// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

// INCLUDED METHODS OF SYSTEM MANAGER
INLINE EntityManager& System::world()
{
    return *m_world;
}

INLINE EventManager& System::dispatcher()
{
    return *m_dispatcher;
}

INLINE SystemManager& System::systems()
{
    return *m_systems;
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
    if( !ent.has_component<D>() )
        ent.add_component<D>();
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
    return m_dispatcher;
}

INLINE EntityManager& SystemManager::world()
{
    return m_world;
}

template<typename S, typename ... Args>
S* SystemManager::ensure(Args && ... args)
{
    auto found = m_systems.find(S::type());
    if( found == m_systems.end() )
    {
        auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
        m_systems.insert(std::make_pair(S::type(), sys));
        return sys;
    }

    return found->second;
}


template<typename S, typename ... Args>
S* SystemManager::add(Args && ... args)
{
    auto found = m_systems.find(S::type());
    ASSERT( found == m_systems.end(), "[ECS] duplicated system.");

    auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
    sys->m_world = &m_world;
    sys->m_dispatcher = &m_dispatcher;
    sys->m_systems = this;

    m_systems.insert(std::make_pair(S::type(), sys));
    return sys;
}

template<typename S>
void SystemManager::remove()
{
    auto found = m_systems.find(S::type());
    if( found != m_systems.end() )
    {
        delete found->second;
        m_systems.erase(found);
    }
}

template<typename S>
S* SystemManager::get()
{
    auto found = m_systems.find(S::type());
    if( found != m_systems.end() )
        return static_cast<S*>(found->second);
    return nullptr;
}