// @date 2016/06/04
// @author Mao Jingkai(oammix@gmail.com)

INLINE EntityManager& Context::world()
{
    return m_world;
}

INLINE const EntityManager& Context::world() const
{
    return m_world;
}

INLINE EventManager& Context::dispatcher()
{
    return m_dispatcher;
}

INLINE const EventManager& Context::dispatcher() const
{
    return m_dispatcher;
}

INLINE SystemManager& Context::system()
{
    return m_system;
}

INLINE const SystemManager& Context::system() const
{
    return m_system;
}