// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/core.hpp>
#include <resource/path.hpp>
#include <math/string_hash.hpp>

#include <list>
#include <fstream>
#include <vector>

NS_LEMON_RESOURCE_BEGIN

struct Resource
{
    using weak_ptr = std::weak_ptr<Resource>;
    using ptr = std::shared_ptr<Resource>;
    virtual ~Resource() {}

    // load/save resource from stream. might be called from a worker thread,
    // return true if successful
    virtual bool read(std::istream&) = 0;
    virtual bool save(std::ostream&) = 0;
    virtual size_t get_memory_usage() const { return 0; }

    virtual bool initialize() { return true; }
    virtual void dispose() {}

    void set_name(const char* str) { _name = str; }
    const std::string& get_name() const { return _name; }

protected:
    std::string _name;
};

namespace details
{
    struct ResourceResolver
    {
        virtual Resource::ptr create() = 0;
    };
}

// resource cache subsystems. loads resources on demand and cache them
// for later access with a LRU strategy.
struct ResourceCache : public core::Subsystem
{
    ResourceCache(size_t threshold = kCacheDefaultThreshold);
    ~ResourceCache();

    bool initialize() override;
    void dispose() override;

    template <typename T> using return_type = typename std::enable_if<
        std::is_base_of<Resource, T>::value,
        std::shared_ptr<T>>::type;

    // return whether a resource exists by name
    bool is_exist(const fs::Path&) const;
    // return a resource with specified type by name, return nullptr if not exists
    template<typename T> return_type<T> get(const fs::Path&);
    // add a mannually constructed resource to cache, returns true if successfully
    bool add(const fs::Path&, Resource::ptr);

    // template<typename T> std::future<return_type<T>> get_async(const fs::Path&);
    // precache resource by name if there are enough spare space in cache,
    // return true if successful
    // template<typename T> bool precache(const fs::Path&);

    size_t get_memory_usage() const { return _memusage; }

protected:
    friend std::ostream& operator << (std::ostream&, const ResourceCache&);

    template<typename T> details::ResourceResolver* get_resolver();

    std::fstream get_file(const fs::Path&);
    Resource::ptr get_internal(details::ResourceResolver*, const fs::Path&);
    void make_room(size_t);
    void touch(math::StringHash);

    size_t _memusage;
    size_t _threshold;

    std::unordered_map<math::StringHash, std::string> _names;
    std::unordered_map<math::StringHash, Resource::ptr> _resources;
    std::list<std::pair<math::StringHash, Resource::ptr>> _lru;
    std::vector<details::ResourceResolver*> _resolvers;
};

std::ostream& operator << (std::ostream&, const ResourceCache&);

/// IMPLEMENTATIONS
namespace details
{
    template<typename T> struct ResourceResolverT : public ResourceResolver
    {
        Resource::ptr create() override;
    };

    template<typename T> Resource::ptr ResourceResolverT<T>::create()
    {
        return Resource::ptr(new (std::nothrow) T());
    }
}

template<typename T> details::ResourceResolver* ResourceCache::get_resolver()
{
    const auto index = TypeInfo::id<Resource, T>();
    if( _resolvers.size() <= index )
        _resolvers.resize(index+1, nullptr);
    
    if( _resolvers[index] == nullptr )
        _resolvers[index] = new (std::nothrow) details::ResourceResolverT<T>();
    
    return _resolvers[index];
}

template<typename T>
ResourceCache::return_type<T> ResourceCache::get(const fs::Path& name)
{
    return std::dynamic_pointer_cast<T>(get_internal(get_resolver<T>(), name));
}

NS_LEMON_RESOURCE_END
