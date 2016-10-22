// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/resource.hpp>
#include <resource/archives.hpp>
#include <engine/arguments.hpp>

NS_LEMON_RESOURCE_BEGIN

std::fstream Resource::search_file(const fs::Path& path)
{
    if( core::details::status() != core::details::Status::RUNNING )
    {
        LOGW("failed to search file %s.", path.c_str());
        return std::fstream();
    }

    auto stream = core::get_subsystem<ArchiveCollection>()->open(path, fs::FileMode::READ);
    if( !stream.is_open() )
        LOGW("failed to open file %s.", path.c_str());

    return stream;
}

bool ResourceCache::initialize()
{
    ENSURE( core::has_subsystems<ArchiveCollection>() );

    auto arguments = core::get_subsystem<Arguments>();

    _memory_threshold = arguments->fetch("/Resource/CacheMemoryThresholdInMB", 64).GetInt();
    _video_memory_threshold = arguments->fetch("/Resource/CacheVideoMemoryThresholdInMB", 64).GetInt();

    // convert into bytes
    _memory_threshold *= (1024 * 1024);
    _video_memory_threshold *= (1024 * 1024);

    _memory_usage = 0;
    _video_memory_usage = 0;
    return true;
}

void ResourceCache::dispose()
{
}

bool ResourceCache::add(math::StringHash hash, Resource::ptr resource)
{
    if( resource == nullptr )
        return false;

    auto found = _resources.find(hash);
    if( found != _resources.end() )
        return false;

    {
        std::unique_lock<std::mutex> L(_mutex);
        make_room(resource);
        _resources[hash] = resource;
        _lru.push_back(std::make_pair(hash, resource));
    }
    return true;
}

void ResourceCache::make_room(Resource::ptr resource)
{
    _memory_usage += resource->get_memory_usage();
    _video_memory_usage += resource->get_video_memroy_usage();

    if( _memory_usage <= _memory_threshold && _video_memory_usage <= _video_memory_threshold )
        return;

    for( auto cursor = _lru.begin(); cursor != _lru.end(); )
    {
        // if this resource are referenced by resource table and least-recently-used list only
        if( cursor->second.use_count() == 2 )
        {
            _memory_usage -= cursor->second->get_memory_usage();
            _video_memory_usage -= cursor->second->get_video_memroy_usage();

            _resources.erase(cursor->first);
            _lru.erase(cursor++);

            if( _memory_usage <= _memory_threshold && _video_memory_usage <= _video_memory_threshold )
                return;
        }
        else
            cursor ++;
    }

    LOGW("failed to get spare memory in ResourceCache.\n\tRAM: %.2f/%.2f mb\n\tVRAM: %.2f/%.2f mb",
        (float)_memory_usage / 1024.f / 1024.f,
        (float)_memory_threshold / 1024.f / 1024.f,
        (float)_video_memory_usage / 1024.f / 1024.f,
        (float)_video_memory_threshold / 1024.f / 1024.f);
}

void ResourceCache::touch(math::StringHash hash)
{
    auto found = std::find_if(_lru.begin(), _lru.end(),
        [=](const std::pair<math::StringHash, Resource::ptr>& pair)
        {
            return pair.first == hash;
        });

    if( found != _lru.end() )
    {
        std::unique_lock<std::mutex> L(_mutex);
        _lru.push_back(*found);
        _lru.erase(found);
    }
}

std::ostream& operator << (std::ostream& out, const ResourceCache& cache)
{
    out << "ResourceCache" << std::endl;

    size_t usage = 0;
    for( auto pair : cache._resources )
    {
        usage += pair.second->get_memory_usage();

        out << "\t" << pair.second->get_name()
            << " REF("<<pair.second.use_count() << "): RAM "
            << pair.second->get_memory_usage() << " byte(s), VRAM "
            << pair.second->get_video_memroy_usage() << " byte(s)." << std::endl;
    }

    return out << usage << " byte(s)" <<std::endl;
}

NS_LEMON_RESOURCE_END
