template<typename ... Args>
bool SubsystemWithEntities<Args...>::initialize()
{
    for( auto object : find_entities_with<Args...>() )
        _entities[object] = get_components<Args...>(object);
    subscribe<EvtEntityModified>(*this);
    return true;
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::dispose()
{
    unsubscribe<EvtEntityModified>(*this);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::receive(const EvtEntityModified& evt)
{
    // auto& world = _context.get_world();
    auto mask = get_components_mask<Args...>();
    if( (get_components_mask(evt.object) & mask) == mask )
        _entities[evt.object] = get_components<Args...>(evt.object);
    else
        _entities.erase(evt.object);
}

template<typename Func, typename Tup, std::size_t... index>
void visit_with_unpack(Func&& cb, Entity object, Tup&& tuple, integer_sequence<size_t, index...>)
{
    return cb(object, *std::get<index>(std::forward<Tup>(tuple))...);
}

template<typename ... Args>
void SubsystemWithEntities<Args...>::visit(const visitor& cb)
{
    constexpr auto Size = std::tuple_size<typename std::decay<tuple>::type>::value;
    for( auto pair : *this )
        visit_with_unpack(cb, pair.first, pair.second, make_integer_sequence<size_t, Size>{});
}

Subsystem* get_subsystem(TypeInfo::index_type);
void add_subsystem(TypeInfo::index_type, Subsystem*);
void remove_subsystem(TypeInfo::index_type);
bool has_subsystem(TypeInfo::index_type);

template<typename S> S& get_subsystem()
{
    auto found = get_subsystem(TypeInfo::id<Subsystem, S>());
    if( found != nullptr )
        return *static_cast<S*>(found);

    FATAL( "trying to get unregistered subsystem: %s.", S::name );
    return *static_cast<S*>(nullptr); // make compiler happy
}

template<typename S, typename ... Args> S& add_subsystem(Args&& ... args)
{
    auto index = TypeInfo::id<Subsystem, S>();
    ASSERT( !has_subsystem(index), "duplicated subsystem: %s.", S::name );

    auto sys = new (std::nothrow) S(std::forward<Args>(args) ...);
    ASSERT( sys->initialize(), "failed to initialize subsystem: %s.", S::name );

    add_subsystem(index, sys);
    return *sys;
}

template<typename S> void remove_subsystem()
{
    auto index = TypeInfo::id<Subsystem, S>();
    remove_subsystem(index);
}

template<typename S> bool has_subsystems()
{
    auto index = TypeInfo::id<Subsystem, S>();
    return has_subsystem(index);
}

template<typename S1, typename S2, typename ... Args> bool has_subsystems()
{
    return has_subsystems<S1>() | has_subsystems<S2, Args...>();
}