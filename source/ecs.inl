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

template<typename ... T>
std::tuple<ComponentHandle<T> ...> Entity::get_components()
{
    assert_valid();
    return m_manager->get_components<T...>(m_identifier);
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

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
INLINE void EntityIterator::next()
{
    while( m_current_cursor < m_capacity && !predicate() )
        m_current_cursor++;

    if( m_current_cursor >= m_capacity )
        m_current_cursor = m_capacity;
}

INLINE bool EntityIterator::predicate() const
{
    return m_manager->is_valid(m_manager->make_uid(m_current_cursor), m_mask);
}

INLINE bool EntityIterator::is_finished() const
{
    return m_current_cursor >= m_capacity;
}

INLINE EntityIterator& EntityIterator::operator ++ ()
{
    m_current_cursor ++;
    next();
    return *this;
}

INLINE bool EntityIterator::operator == (const EntityIterator& rh) const
{
    return m_current_cursor == rh.m_current_cursor;
}

INLINE bool EntityIterator::operator != (const EntityIterator& rh) const
{
    return m_current_cursor != rh.m_current_cursor;
}

INLINE Entity EntityIterator::operator * ()
{
    return Entity(m_manager, m_manager->make_uid(m_current_cursor));
}

INLINE EntityIterator EntityView::begin()
{
    return EntityIterator(m_manager, m_mask, 0);
}

INLINE EntityIterator EntityView::end()
{
    return EntityIterator(m_manager, m_mask, uint32_t(m_manager->capacity()));
}

INLINE const EntityIterator EntityView::begin() const
{
    return EntityIterator(m_manager, m_mask, 0);
}

INLINE const EntityIterator EntityView::end() const
{
    return EntityIterator(m_manager, m_mask, uint32_t(m_manager->capacity()));
}

template<typename ... T>
void EntityViewTrait<T...>::each(std::function<void(Entity, T& ...)> closure)
{
    for(auto cursor : *this)
        closure(cursor, *(cursor.template get_component<T>().get())...);
}

template<typename ... T>
EntityViewTrait<T...>::EntityViewTrait(EntityManager* manager)
: EntityView(manager, manager->get_components_mask<T...>())
{}

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
T* ComponentHandle<T>::get()
{
    assert_valid();
    return m_manager->get_component_ptr<T>(m_ent);
}

template<typename T>
const T* ComponentHandle<T>::get() const
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
Component::Class ComponentTrait<T>::get_class()
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
            if( p ) p->resize(index+1);
    }
}

template<typename T>
ObjectChunksTrait<T>* EntityManager::get_chunks()
{
    const auto cls = ComponentTrait<T>::get_class();
    if( m_components_pool.size() < (cls+1) )
        m_components_pool.resize((cls+1), nullptr);

    if( m_components_pool[cls] == nullptr )
    {
        auto chunks = new ObjectChunksTrait<T>();
        chunks->resize(m_index_counter);
        m_components_pool[cls] = chunks;
    }

    return static_cast<ObjectChunksTrait<T>*>(m_components_pool[cls]);
}

INLINE bool EntityManager::is_valid(Entity::Uid id) const
{
    return id.index() < m_components_mask.size() && m_versions[id.index()] == id.version();
}

INLINE bool EntityManager::is_valid(Entity::Uid id, const ComponentMask mask) const
{
    return is_valid(id) && (m_components_mask[id.index()] & mask) == mask;
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

INLINE size_t EntityManager::size() const
{
    return m_versions.size() - m_freeslots.size();
}

INLINE size_t EntityManager::capacity() const
{
    return m_versions.size();
}

INLINE Entity::Uid EntityManager::make_uid(uint32_t index)
{
    return Entity::Uid(index, m_versions[index]);
}

template<typename T, typename ... Args>
ComponentHandle<T> EntityManager::add_component(Entity::Uid id, Args&& ... args)
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();
    assert(!m_components_mask[id.index()].test(cls) && "[ECS] duplicated component to Entity.");

    // placement new into the component pool
    get_chunks<T>()->construct(id.index(), std::forward<Args>(args) ...);
    // set the bit mask for this component
    m_components_mask[id.index()].set(cls);

    auto handle = ComponentHandle<T>(this, id);
    return handle;
}

template<typename T>
void EntityManager::remove_component(Entity::Uid id)
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();
    const uint32_t index = id.index();

    if( m_components_mask[id.index()].test(cls) )
    {
        // remove the bit mask for this component
        m_components_mask[id.index()].reset(cls);
        // call destructor
        get_chunks<T>()->destruct(id.index());
    }
}

template<typename T>
bool EntityManager::has_component(Entity::Uid id) const
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();
    return m_components_mask[id.index()].test(cls);
}

template<typename T>
ComponentHandle<T> EntityManager::get_component(Entity::Uid id)
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();

    if( cls >= m_components_pool.size() )
        return ComponentHandle<T>();

    return ComponentHandle<T>(this, id);
}

template<typename ... T>
std::tuple<ComponentHandle<T>...> EntityManager::get_components(Entity::Uid id)
{
    return std::make_tuple(get_component<T>(id)...);
}

template<typename T>
T* EntityManager::get_component_ptr(Entity::Uid id)
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();

    if( cls >= m_components_pool.size() )
        return nullptr;

    return static_cast<T*>(m_components_pool[cls]->get(id.index()));
}

template<typename T>
const T* EntityManager::get_component_ptr(Entity::Uid id) const
{
    assert_valid(id);

    const auto cls = ComponentTrait<T>::get_class();

    if( cls >= m_components_pool.size() )
        return nullptr;

    return static_cast<const T*>(m_components_pool[cls]->get(id.index()));
}

INLINE ComponentMask EntityManager::get_components_mask(Entity::Uid id) const
{
    assert_valid(id);

    return m_components_mask[id.index()];
}

template<typename T>
ComponentMask EntityManager::get_components_mask() const
{
    ComponentMask mask;
    mask.set(ComponentTrait<T>::get_class());
    return mask;
}

template<typename T1, typename T2, typename ... Args>
ComponentMask EntityManager::get_components_mask() const
{
    return get_components_mask<T1>() | get_components_mask<T2, Args ...>();
}

template<typename ... T>
EntityViewTrait<T...> EntityManager::find_entities_with()
{
    return EntityViewTrait<T...>(this);
}

INLINE EntityView EntityManager::get_entities()
{
    return EntityView(this);
}