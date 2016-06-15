#pragma once

#include <flow2d/forward.hpp>

NS_FLOW2D_BEGIN

union ResourceId
{
    struct {
        uint32_t index : 32;
        uint32_t version : 24;
        uint32_t type : 8;
    };

    uint64_t value;

    ResourceId();
    ResourceId(uint32_t, uint32_t, uint32_t);
    bool operator == (const ResourceId& rh);
};

template<typename T> struct ResourceHandle
{
    T* operator -> ();
    const T* operator -> () const;
    operator bool() const;
    bool operator == (const ResourceHandle&);

    void        retain();
    void        release();

    bool        is_valid() const;
    ResourceId  get_resource_id() const;
    size_t      get_refcount() const;

protected:
    friend class ResourceCacheManager;
    ResourceHandle(ResourceCacheManager*, ResourceId);

    ResourceId              rid;
    ResourceCacheManager*   cache;
};

struct Resource
{
    using RuntimeTypeId = size_t;

    virtual ~Resource() {}
    virtual bool    load_from_file(std::unique_ptr<DataStream>) = 0;
    virtual size_t  get_memory_usage() const = 0;

protected:
    static RuntimeTypeId s_rti;
};

template<typename T> struct ResourceTrait : public Resource
{
    using resource_type = T;
    static RuntimeTypeId type();

    virtual ~ResourceTrait() {}
};

struct ResourceCacheManager
{
    ResourceCacheManager(ArchiveManager&, size_t threshold = kCacheDefaultThreshold);
    ~ResourceCacheManager();

    template<typename T> ResourceHandle<typename T::resource_type>
    get(const char*);

    template<typename T> T* get_ptr(ResourceId);
    template<typename T> const T* get_ptr(ResourceId) const;

    void    retain(ResourceId);
    void    release(ResourceId);

    bool    is_valid(ResourceId) const;
    size_t  get_refcount(ResourceId) const;
    size_t  get_memory_usage() const;

protected:
    void touch(ResourceId);
    void accomodate_storage(size_t);
    ResourceId try_insert(Resource*, const char*, uint32_t);
    void try_make_room(size_t);

    ArchiveManager&         m_archives;
    std::unordered_map<std::string, ResourceId> m_identifiers;
    size_t m_index_counter = 0;

    //
    std::vector<Resource*>  m_resources;
    //
    size_t                  m_memory_usage;
    size_t                  m_cache_threshold;
    std::list<ResourceId>   m_lru;
    // refcount of resource, it could be free if refcount equals zero
    std::vector<size_t>     m_refcounts;
    // slot version number. incremented each time an resource is destroyed
    std::vector<size_t>     m_versions;
    // list of available resource slots
    std::vector<size_t>     m_freeslots;
};

#include <flow2d/core/resource.inl>
NS_FLOW2D_END

namespace std
{
    template<> struct hash<flow2d::ResourceId>
    {
        size_t operator()(const flow2d::ResourceId& id) const
        {
            return hash<uint64_t>()(id.value);
        }
    };
}