// INCLUDED METHODS OF ENTITY
INLINE bool Entity::is_valid() const
{
    return m_manager && m_manager->is_valid(m_identifier);
}

INLINE void Entity::assert_valid() const
{
    assert(is_valid() && "[ECS] trying to access invalid Entity.");
}

INLINE Entity::operator bool() const
{
    return is_valid();
}

INLINE bool Entity::operator == (const Entity& rh) const
{
    return m_manager == rh.m_manager && m_identifier == rh.m_identifier;
}

INLINE bool Entity::operator != (const Entity& rh) const
{
    return !(rh == *this);
}

INLINE bool Entity::operator < (const Entity& rh) const
{
    return m_identifier < rh.m_identifier;
}

template<typename T, typename ... Args>
ComponentHandle<T> Entity::add_component(Args && ... args)
{
    assert_valid();
    return m_manager->add_component<T>(m_identifier, std::forward<Args>(args)...);
}

template<typename T>
ComponentHandle<T> Entity::add_component(const T& rh)
{
    assert_valid();
    return m_manager->add_component<T>(m_identifier, std::forward<const T&>(rh));
}

template<typename T>
bool Entity::has_component() const
{
    assert_valid();
    return m_manager->has_component<T>(m_identifier);
}

template<typename T>
ComponentHandle<T> Entity::get_component()
{
    assert_valid();
    return m_manager->get_component<T>(m_identifier);
}

template<typename T>
void Entity::remove_component()
{
    assert_valid();
    m_manager->remove_component<T>(m_identifier);
}

INLINE Entity::Uid Entity::get_uid() const
{
    return m_identifier;
}

INLINE ComponentMask Entity::get_components_mask() const
{
    assert_valid();
    return m_manager->get_components_mask(m_identifier);
}

INLINE void Entity::invalidate()
{
    m_identifier = INVALID;
    m_manager = nullptr;
}

INLINE void Entity::dispose()
{
    assert_valid();
    m_manager->erase(m_identifier);
    invalidate();
}

// INCLUDED METHODS OF COMPONENT HANDLE
template<typename T>
ComponentHandle<T>::ComponentHandle(EntityManager* manager, Entity::Uid uid)
: m_manager(manager), m_ent(uid){}

template<typename T>
bool ComponentHandle<T>::is_valid() const
{
    return m_manager && m_manager->is_valid(m_ent) && m_manager->has_component<T>(m_ent);
}

template<typename T>
void ComponentHandle<T>::assert_valid() const
{
    assert(is_valid());
}

template<typename T>
ComponentHandle<T>::operator bool() const
{
    return is_valid();
}

template<typename T>
void ComponentHandle<T>::dispose()
{
    assert_valid();
    m_manager->remove_component<T>(m_ent);
}

template<typename T>
Entity ComponentHandle<T>::entity()
{
    assert_valid();
    return m_manager->get(m_ent);
}

template<typename T>
T* ComponentHandle<T>::operator -> ()
{
    assert_valid();
    return m_manager->get_component_ptr<T>(m_ent);
}

template<typename T>
const T* ComponentHandle<T>::operator -> () const
{
    assert_valid();
    return m_manager->get_component_ptr<T>(m_ent);
}

template<typename T>
bool ComponentHandle<T>::operator == (const ComponentHandle<T>& rh) const
{
    return m_manager == rh.m_manager && m_ent == rh.m_ent;
}

template<typename T>
bool ComponentHandle<T>::operator != (const ComponentHandle<T>& rh) const
{
    return !(*this == rh);
}

// INCLUDED METHODS OF COMPONENT TRAITS
template<typename T>
ComponentTraits::Class Component<T>::get_class()
{
    static Class cls = s_class_counter ++;
    assert(cls < kEntMaxComponents);
    return cls;
}

// INCLUDED METHODS OF ENTITY MANAGER
INLINE void EntityManager::accomodate_entity(uint32_t index)
{
    if( m_components_mask.size() < (index+1) )
    {
        m_components_mask.resize(index+1);
        m_versions.resize(index+1);
        for( auto p : m_components_pool )
            if( p ) p->expand(index+1);
    }
}

template<typename T>
Pool<T>* EntityManager::accomodate_component()
{
    const ComponentTraits::Class cls = Component<T>::get_class();
    if( m_components_pool.size() < (cls+1) )
        m_components_pool.resize((cls+1), nullptr);

    if( m_components_pool[cls] == nullptr )
    {
        auto pool = new Pool<T>();
        pool->expand(m_index_counter);
        m_components_pool[cls] = pool;
    }

    return static_cast<Pool<T>*>(m_components_pool[cls]);
}

INLINE bool EntityManager::is_valid(Entity::Uid id) const
{
    return id.index() < m_components_mask.size() && m_versions[id.index()] == id.version();
}

INLINE void EntityManager::assert_valid(Entity::Uid id) const
{
    assert( id.index() < m_components_mask.size() && "[ECS] Entity::Uid ID outside entity vector range." );
    assert( m_versions[id.index()] == id.version() && "[ECS] attempt to access Entity via a stale Entity::Uid.");
}

INLINE Entity EntityManager::get(Entity::Uid id)
{
    return Entity(this, id);
}

INLINE size_t EntityManager::size()
{
    return m_versions.size() - m_freeslots.size();
}

template<typename T, typename ... Args>
ComponentHandle<T> EntityManager::add_component(Entity::Uid id, Args&& ... args)
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();
    assert(!m_components_mask[id.index()].test(cls) && "[ECS] duplicated component to Entity.");

    // placement new into the component pool
    auto pool = accomodate_component<T>();
    ::new(pool->get(id.index())) T(std::forward<Args>(args) ...);

    // set the bit mask for this component
    m_components_mask[id.index()].set(cls);

    auto handle = ComponentHandle<T>(this, id);
    return handle;
}

template<typename T>
void EntityManager::remove_component(Entity::Uid id)
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();
    const uint32_t index = id.index();

    if( m_components_mask[id.index()].test(cls) )
    {
        // remove the bit mask for this component
        m_components_mask[id.index()].reset(cls);

        // call destructor
        auto pool = accomodate_component<T>();
        pool->erase(index);
    }
}

template<typename T>
bool EntityManager::has_component(Entity::Uid id)
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();

    return m_components_mask[id.index()].test(cls);
}

template<typename T>
ComponentHandle<T> EntityManager::get_component(Entity::Uid id)
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();

    if( cls >= m_components_pool.size() )
        return ComponentHandle<T>();

    return ComponentHandle<T>(this, id);
}

template<typename T>
T* EntityManager::get_component_ptr(Entity::Uid id)
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();

    if( cls >= m_components_pool.size() )
        return nullptr;

    return static_cast<T*>(m_components_pool[cls]->get(id.index()));
}

template<typename T>
const T* EntityManager::get_component_ptr(Entity::Uid id) const
{
    assert_valid(id);

    const ComponentTraits::Class cls = Component<T>::get_class();

    if( cls >= m_components_pool.size() )
        return nullptr;

    return static_cast<const T*>(m_components_pool[cls]->get(id.index()));
}

INLINE ComponentMask EntityManager::get_components_mask(Entity::Uid id)
{
    assert_valid(id);

    return m_components_mask[id.index()];
}
