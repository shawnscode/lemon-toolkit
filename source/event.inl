// INCLUDED METHODS OF EVENT DISPATCHER
INLINE void EventDispatcher::unlink(ReceiverLink* link)
{
    link->closure = nullptr;
    if( link->next ) link->next->prev = link->prev;
    if( link->prev ) link->prev->next = link->next;
    delete link;
}

INLINE size_t EventDispatcher::add_before(ReceiverLink* cursor, const EventDispatcher::Closure& c)
{
    auto link = new ReceiverLink(c);
    link->prev = cursor->prev;
    link->next = cursor;
    cursor->prev->next = link;
    cursor->prev = link;

    static_assert( sizeof(link) == sizeof(size_t),
        "[EVENT] size of size_t is different with void*" );
    return (size_t)link;
}

INLINE size_t EventDispatcher::size() const
{
    if( !m_ring ) return 0;

    auto size = 0;
    auto link = m_ring;
    do {
        if( link->closure != nullptr ) size ++;
        link = link->next;
    } while( link != m_ring );
    return size;
}

INLINE size_t EventDispatcher::connect(const EventDispatcher::Closure& c)
{
    if( m_ring == nullptr )
    {
        m_ring = new ReceiverLink(c);
        m_ring->prev = m_ring;
        m_ring->next = m_ring;
        return (size_t)m_ring;
    }

    return add_before(m_ring, c);
}

INLINE bool EventDispatcher::disconnect(size_t id)
{
    if( !m_ring ) return false;

    if( (size_t)m_ring == id )
    {
        ReceiverLink* tmp = nullptr;
        if( m_ring->next != m_ring )
            tmp = m_ring->next;
        unlink(m_ring);
        m_ring = tmp;
        return true;
    }

    for( auto cursor = m_ring->next; cursor != m_ring; cursor = cursor->next )
    {
        if( (size_t)cursor == id )
        {
            unlink(cursor);
            return true;
        }
    }

    return false;
}

INLINE void EventDispatcher::emit(const void* data)
{
    if( !m_ring ) return;

    auto link = m_ring;
    do {
        if( link->closure != nullptr )
            link->closure(data);
        link = link->next;
    } while( link != m_ring );
}

template<typename E, typename R>
void EventManager::subscribe(R& receiver)
{
    auto rid = (size_t)(&receiver);
    static_assert( sizeof(size_t) == sizeof(&receiver),
        "[EVENT] size of size_t is different with void*" );

    auto found = m_records.find(rid);
    if( found == m_records.end() )
        m_records[rid] = SubscripedEvents();

    auto& record = m_records[rid];
    auto cursor = record.find(EventTrait<E>::type());
    assert( cursor == record.end() && "[EVENT] duplicated subscribe to one event." );

    auto dispatcher = get_dispatcher(EventTrait<E>::type());
    auto id = dispatcher->connect([&](const void* event){
        receiver.receive(*static_cast<const E*>(event));
    });
    record[EventTrait<E>::type()] = id;
}

template<typename E, typename R>
bool EventManager::unsubscribe(R& receiver)
{
    auto rid = (size_t)(&receiver);
    static_assert( sizeof(size_t) == sizeof(&receiver),
        "[EVENT] size of size_t is different with void*" );

    auto found = m_records.find(rid);
    if( found == m_records.end() )
        return false;

    found = m_records.find(rid);
    auto cursor = found->second.find(EventTrait<E>::type());
    if( cursor != found->second.end() )
    {
        auto dispatcher = get_dispatcher(EventTrait<E>::type());
        auto ret = dispatcher->disconnect(cursor->second);
        found->second.erase(cursor);

        if( found->second.size() == 0 )
            m_records.erase(found);
        return ret;
    }

    return false;
}

template<typename E>
void EventManager::emit(const E& event)
{
    auto dispatcher = get_dispatcher(EventTrait<E>::type());
    dispatcher->emit(&event);
}

template<typename E, typename ... Args>
void EventManager::emit(Args && ... args)
{
    auto dispatcher = get_dispatcher(EventTrait<E>::type());
    auto event = E(std::forward<Args>(args) ...);
    dispatcher->emit(&event);
}

INLINE EventDispatcher* EventManager::get_dispatcher(Event::Type id)
{
    if( id >= m_dispatchers.size() )
        m_dispatchers.resize(id+1);

    if( m_dispatchers[id] == nullptr )
        m_dispatchers[id] = new EventDispatcher();

    return m_dispatchers[id];
}
