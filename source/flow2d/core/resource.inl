INLINE bool ResourceId::operator == (const ResourceId& rh)
{
    return value == rh.value;
}

template<typename T>
ResourceHandle<T>::ResourceHandle(ResourceCacheManager* cache, ResourceId id)
: cache(cache), rid(id)
{}

template<typename T>
INLINE T* ResourceHandle<T>::operator -> ()
{
    return cache->get_ptr<T>(rid);
}

template<typename T>
INLINE const T* ResourceHandle<T>::operator -> () const
{
    return cache->get_ptr<T>(rid);
}

template<typename T>
INLINE ResourceHandle<T>::operator bool () const
{
    return is_valid();
}

template<typename T>
INLINE bool ResourceHandle<T>::operator == (const ResourceHandle<T>& rh)
{
    return rid == rh.rid && cache == rh.cache;
}

template<typename T>
INLINE void ResourceHandle<T>::retain ()
{
    return cache->retain(rid);
}

template<typename T>
INLINE void ResourceHandle<T>::release ()
{
    return cache->release(rid);
}

template<typename T>
INLINE bool ResourceHandle<T>::is_valid() const
{
    return cache->is_valid(rid);
}

template<typename T>
INLINE ResourceId ResourceHandle<T>::get_resource_id() const
{
    return rid;
}

template<typename T>
INLINE size_t ResourceHandle<T>::get_refcount() const
{
    return cache->get_refcount(rid);
}

template<typename T>
INLINE Resource::RuntimeTypeId ResourceTrait<T>::type()
{
    static Resource::RuntimeTypeId rtid = s_rti++;
    return rtid;
}

INLINE bool ResourceCacheManager::is_valid(ResourceId id) const
{
    return
        id.index < m_resources.size() &&
        id.index < std::numeric_limits<uint32_t>::max() &&
        id.version == m_versions[id.index];
}

INLINE void ResourceCacheManager::retain(ResourceId id)
{
    ASSERT( is_valid(id) , "try to retain a non-existed resource." );

    if( m_refcounts[id.index] == 1 )
    {
        auto cursor = std::find(m_lru.begin(), m_lru.end(), id);
        ENSURE( cursor != m_lru.end() );
        m_lru.erase(cursor);
    }

    m_refcounts[id.index] ++;
}

INLINE void ResourceCacheManager::release(ResourceId id)
{
    ASSERT( is_valid(id), "try to release a non-existed resource." );
    ENSURE( m_refcounts[id.index] > 1 );

    m_refcounts[id.index] --;
    if( m_refcounts[id.index] == 1 )
        m_lru.push_back(id);
}

INLINE size_t ResourceCacheManager::get_refcount(ResourceId id) const
{
    if( !is_valid(id) )
        return 0;
    return m_refcounts[id.index];
}

INLINE void ResourceCacheManager::accomodate_storage(size_t index)
{
    if( m_resources.size() < (index + 1) )
    {
        m_resources.resize(index+1);
        m_versions.resize(index+1);
        m_refcounts.resize(index+1);
    }
}

template<typename T>
ResourceHandle<typename T::resource_type> ResourceCacheManager::get(const char* name)
{
    auto cursor = m_identifiers.find(name);
    if( cursor != m_identifiers.end() )
    {
        if( is_valid(cursor->second) )
        {
            touch(cursor->second);
            return { this, cursor->second };
        }
    }

    uint32_t index, version;
    if( m_freeslots.empty() )
    {
        index = m_index_counter ++;
        accomodate_storage(index);
        version = m_versions[index] = 1;
    }
    else
    {
        index = m_freeslots.back();
        m_freeslots.pop_back();
        version = m_versions[index];
    }

    auto id = ResourceId(index, version, T::type() );
    m_identifiers[name] = id;

    auto resource = new T();
    if( !resource->load_from_file(name) )
    {
        delete resource;
        return { this, ResourceId() };
    }

    m_resources[index] = resource;
    m_refcounts[index] = 1;
    m_memory_usage += resource->get_memory_usage();

    if( m_memory_usage > m_cache_threshold )
        try_make_room( (m_memory_usage - m_cache_threshold)*1.25f );

    m_lru.push_back(id);
    return { this, id };
}

template<typename T>
INLINE T* ResourceCacheManager::get_ptr(ResourceId id)
{
    if( !is_valid(id) || T::type() != id.type )
        return nullptr;

    return static_cast<T*>(m_resources[id.index]);
}

template<typename T>
INLINE const T* ResourceCacheManager::get_ptr(ResourceId id) const
{
    if( !is_valid(id) || T::type() != id.type )
        return nullptr;

    return static_cast<T*>(m_resources[id.index]);
}