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
#include <mutex>

NS_LEMON_RESOURCE_BEGIN

struct Resource
{
    using weak_ptr = std::weak_ptr<Resource>;
    using ptr = std::shared_ptr<Resource>;

    template<typename T>
    using derived = typename std::enable_if<std::is_base_of<Resource, T>::value, T>;
    template<typename T>
    using shared_derived_ptr = typename std::shared_ptr<typename derived<T>::type>;

    template<typename T, typename ... Args>
    static shared_derived_ptr<T> create(Args&& ... args);
    template<typename T>
    static shared_derived_ptr<T> read(const fs::Path&);

public:
    virtual ~Resource() {}

    // load/save resource from stream. might be called from a worker thread,
    // return true if successful
    virtual bool read(std::istream&) = 0;
    virtual bool save(std::ostream&) = 0;

    // returns memory/video memory consumptions of this resource
    virtual size_t get_memory_usage() const { return 0;}
    virtual size_t get_video_memroy_usage() const { return 0; }

    // setter/getter of name
    void set_name(const char* str) { _name = str; }
    const std::string& get_name() const { return _name; }

protected:
    bool initialize() { return true; }
    static std::fstream search_file(const fs::Path&);

protected:
    std::string _name;
};

// resource cache subsystems. loads resources on demand and cache them
// for later access with a LRU strategy.
struct ResourceCache : public core::Subsystem
{
    bool initialize() override;
    void dispose() override;

    // returns whether a resource exists
    bool is_exist(const fs::Path&) const;
    // add resource to cache, returns true if successfully
    bool add(const fs::Path&, Resource::ptr);
    bool add(math::StringHash, Resource::ptr);
    // find resource associated with name and returns it
    template<typename T> Resource::shared_derived_ptr<T> find(const fs::Path&);
    template<typename T> Resource::shared_derived_ptr<T> find(math::StringHash);
    // if resource associated with name not exists in cache, then try to read it from a
    template<typename T> Resource::shared_derived_ptr<T> fetch(const fs::Path&);
    // returns memory/video usage of cached resource
    size_t get_memory_usage() const;
    size_t get_video_memroy_usage() const;
    // set threshold of cache
    void set_threshold(size_t, size_t);

protected:
    friend std::ostream& operator << (std::ostream&, const ResourceCache&);

    void make_room(Resource::ptr);
    void touch(math::StringHash);

    std::mutex _mutex;
    size_t _memory_usage;
    size_t _video_memory_usage;
    size_t _memory_threshold;
    size_t _video_memory_threshold;

    std::unordered_map<math::StringHash, Resource::ptr> _resources;
    std::list<std::pair<math::StringHash, Resource::ptr>> _lru;
};

std::ostream& operator << (std::ostream&, const ResourceCache&);

/// IMPLEMENTATIONS

template<typename T, typename ... Args>
Resource::shared_derived_ptr<T> Resource::create(Args&& ... args)
{
    auto v = new (std::nothrow) T();
    if( v && v->initialize(std::forward<Args>(args)...) ) return v;
    if( v ) delete v;
    return nullptr;
}

template<typename T>
Resource::shared_derived_ptr<T> Resource::read(const fs::Path& name)
{
    auto v = new (std::nothrow) T();
    auto fstream = Resource::search_file(name);
    if( v && v->read(fstream) ) return shared_derived_ptr<T>(v);
    if( v ) delete v;
    return nullptr;
}

template<typename T>
Resource::shared_derived_ptr<T> ResourceCache::find(math::StringHash hash)
{
    auto found = _resources.find(hash);
    if( found == _resources.end() )
        return nullptr;
    touch(hash);
    return std::dynamic_pointer_cast<T>(found->second);
}

template<typename T>
Resource::shared_derived_ptr<T> ResourceCache::find(const fs::Path& name)
{
    auto hash = math::StringHash(name.c_str());
    return find<T>(hash);
}

template<typename T>
Resource::shared_derived_ptr<T> ResourceCache::fetch(const fs::Path& name)
{
    auto hash = math::StringHash(name.c_str());

    if( auto resource = find<T>(hash) )
        return resource;

    if( auto resource = Resource::read<T>(name) )
    {
        resource->set_name(name.c_str());
        return add(hash, resource) ? resource : nullptr;
    }

    return nullptr;
}

INLINE bool ResourceCache::is_exist(const fs::Path& path) const
{
    return _resources.find(path.c_str()) != _resources.end();
}

INLINE bool ResourceCache::add(const fs::Path& name, Resource::ptr resource)
{
    if( resource != nullptr )
        resource->set_name(name.c_str());
    return add(math::StringHash(name.c_str()), resource);
}

INLINE size_t ResourceCache::get_memory_usage() const
{
    return _memory_usage;
}

INLINE size_t ResourceCache::get_video_memroy_usage() const
{
    return _video_memory_usage;
}

INLINE void ResourceCache::set_threshold(size_t memory, size_t video_memory)
{
    _memory_threshold = memory;
    _video_memory_threshold = video_memory;
}

NS_LEMON_RESOURCE_END
