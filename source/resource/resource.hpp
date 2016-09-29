// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/defines.hpp>
#include <resource/path.hpp>
#include <math/string_hash.hpp>
#include <core/subsystem.hpp>

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

    void set_name(const char* str) { _name = str; }
    const std::string& get_name() const { return _name; }
    unsigned get_memusage() const { return _memusage; } // in bytes

protected:
    std::string _name;
    unsigned _memusage;
};

// resource cache subsystems. loads resources on demand and cache them
// for later access with a LRU strategy.
struct ResourceCache : public core::Subsystem
{
    SUBSYSTEM("ResourceCache");

    ResourceCache(unsigned threshold = kCacheDefaultThreshold);
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
    // template<typename T> std::future<return_type<T>> get_async(const fs::Path&);
    // precache resource by name if there are enough spare space in cache,
    // return true if successful
    // template<typename T> bool precache(const fs::Path&);

    unsigned get_memusage() const { return _memusage; }

protected:
    friend std::ostream& operator << (std::ostream&, const ResourceCache&);

    void make_room(unsigned);
    void touch(math::StringHash);
    std::fstream get_file(const fs::Path&);

    unsigned _memusage;
    unsigned _threshold;

    std::unordered_map<math::StringHash, std::string>     _names;
    std::unordered_map<math::StringHash, Resource::ptr>   _resources;
    std::list<std::pair<math::StringHash, Resource::ptr>> _lru;
};

std::ostream& operator << (std::ostream&, const ResourceCache&);

/// IMPLEMENTATIONS
template<typename T>
ResourceCache::return_type<T> ResourceCache::get(const fs::Path& name)
{
    auto hash = math::StringHash(name.c_str());
    auto found = _resources.find(hash);
    if( found != _resources.end() )
    {
        touch(hash);
        return  std::dynamic_pointer_cast<T>(found->second);
    }

    auto file = get_file(name);
    if( !file.is_open() )
    {
        LOGW("failed to get due to file not exists, \"%s\"", name.c_str());
        return nullptr;
    }

    auto resource = std::shared_ptr<T>(new (std::nothrow) T());
    if( resource )
    {
        resource->set_name(name.c_str());
        if( resource->read(file) )
        {
            make_room(resource->get_memusage());
            _resources[hash] = resource;
            _lru.push_back(std::make_pair(hash, resource));
            _names[hash] = name.to_string();
            return resource;
        }
    }

    return nullptr;
}

NS_LEMON_RESOURCE_END