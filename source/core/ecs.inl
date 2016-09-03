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

template<typename T> void register_component()
{
    MARK_AS_MAIN_THREAD_ONLY();

    auto destructor = [](Entity object, void* chunk)
    {
        T* component = static_cast<T*>(chunk);
        component->dispose();
        component->~T();
    };

    const auto id = TypeInfo::id<Component, T>();
    return ecs::register_component(id, sizeof(T), T::chunk_size, destructor);
}

template<typename T, typename ... Args> T* add_component(Entity object, Args&& ... args)
{
    ASSERT( alive(object), "invalid operation: dead entity." );
    ASSERT( !has_components<T>(object), "invalid operation: duplicated component." );

    const auto id = TypeInfo::id<Component, T>();
    auto component = static_cast<T*>(ecs::add_component(id, object));
    if( component == nullptr )
        return nullptr;

    ::new(component) T(std::forward<Args>(args)...);
    if( !component->initialize() )
    {
        ecs::remove_component(object, id);
        return nullptr;
    }

    return component;
}

template<typename T> INLINE T* get_component(Entity object)
{
    const auto id = TypeInfo::id<Component, T>();
    return static_cast<T*>(ecs::get_component(id, object));
}

template<typename ... T> INLINE std::tuple<T*...> get_components(Entity object)
{
    return std::make_tuple(get_component<T>(object)...);
}

template<typename T> INLINE void remove_component(Entity object)
{
    const auto id = TypeInfo::id<Component, T>();
    ecs::remove_component(id, object);
}

template<> INLINE bool has_components(Entity object)
{
    return true;
}

template<typename ... Args> INLINE bool has_components(Entity object)
{
    return ecs::has_components_t<Args ...>();
}

template<> INLINE ComponentMask get_components_mask()
{
    return ComponentMask();
}

template<typename ... Args> INLINE ComponentMask get_components_mask()
{
    return ecs::get_components_mask_t<Args...>();
}

namespace ecs
{
    template<typename T>
    INLINE bool has_components_t(Entity object)
    {
        const auto id = TypeInfo::id<Component, T>();
        return has_component(id, object);
    }

    template<typename T1, typename T2, typename ... Args>
    INLINE bool has_components_t(Entity object)
    {
        return has_components_t<T1>(object) & has_components_t<T2, Args...>(object);
    }

    template<typename T>
    INLINE ComponentMask get_components_mask_t()
    {
        ComponentMask mask;
        mask.set(TypeInfo::id<Component, T>());
        return mask;
    }

    template<typename T1, typename T2, typename ... Args>
    INLINE ComponentMask get_components_mask_t()
    {
        return get_components_mask_t<T1>() | get_components_mask_t<T2, Args ...>();
    }

    // INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
    INLINE iterator::iterator(Entity::index_type index, ComponentMask mask)
    : _index(index), _mask(mask)
    {
        _index = ecs::find_next_available(_index, _mask, true);
    }

    INLINE iterator& iterator::operator ++ ()
    {
        _index = ecs::find_next_available(_index, _mask);
        return *this;
    }

    INLINE iterator iterator::operator ++ (int dummy)
    {
        auto tmp = *this;
        (*this) ++;
        return tmp;
    }

    INLINE bool iterator::operator == (const iterator& rhs) const
    {
        return _index == rhs._index && _mask == rhs._mask;
    }

    INLINE bool iterator::operator != (const iterator& rhs) const
    {
        return !(*this == rhs);
    }

    INLINE Entity iterator::operator * () const
    {
        return ecs::get(_index);
    }

    template<typename ... Args> view<Args...>::view()
    : _mask(get_components_mask<Args...>())
    {}

    template<typename ... Args> INLINE iterator view<Args...>::begin() const
    {
        return iterator(0, _mask);
    }

    template<typename ... Args> INLINE iterator view<Args...>::end() const
    {
        return iterator(Entity::invalid, _mask);
    }

    template<typename ... Args> INLINE void view<Args...>::visit(const visitor& cb)
    {
        for( auto cursor : *this )
            cb(cursor, *::lemon::core::get_component<Args>(cursor) ...);
    }
}