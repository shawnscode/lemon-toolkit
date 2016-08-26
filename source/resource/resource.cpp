// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/resource.hpp>
#include <resource/archives.hpp>

NS_LEMON_RESOURCE_BEGIN

ResourceCache::ResourceCache(core::Context& ctx, unsigned threshold)
: core::Subsystem(ctx), _threshold(threshold), _memusage(0)
{}

ResourceCache::~ResourceCache()
{}

bool ResourceCache::initialize()
{
    ENSURE( has_subsystems<ArchiveCollection>() );
    return true;
}

void ResourceCache::dispose()
{

}

bool ResourceCache::is_exist(const fs::Path& path) const
{
    return _resources.find(path.c_str()) != _resources.end();
}

void ResourceCache::make_room(unsigned size)
{
    if( _memusage + size <= _threshold )
    {
        _memusage += size;
        return;
    }

    _memusage = size;
    for( auto pair : _resources ) _memusage += pair.second->get_memusage();
    for( auto cursor = _lru.begin(); cursor != _lru.end(); )
    {
        if( cursor->second.use_count() == 2 )
        {
            _memusage -= cursor->second->get_memusage();
            _resources.erase(cursor->first);
            _names.erase(cursor->first);
            _lru.erase(cursor++);

            if( _memusage + size <= _threshold ) return;
        }
        else
            cursor ++;
    }

    LOGW("failed to get spare memory in ResourceCache, %.2f/%.2f mb",
        (float)_memusage / 1024.f / 1024.f,
        (float)_threshold / 1024.f / 1024.f);
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
        _lru.push_back(*found);
        _lru.erase(found);
    }
}

std::fstream ResourceCache::get_file(const fs::Path& path)
{
    return get_subsystem<ArchiveCollection>().open(path, fs::FileMode::READ);
}

std::ostream& operator << (std::ostream& out, const ResourceCache& cache)
{
    out << "ResourceCache" << std::endl;
    unsigned usage = 0;
    for( auto pair : cache._resources )
    {
        usage += pair.second->get_memusage();
        auto name = cache._names.find(pair.first);
        if( name != cache._names.end() ) out << "\t" << name->second;
        else out << "\t" << pair.first;
        out << " : " << pair.second->get_memusage() << " byte(s)" << std::endl;
    }
    return out << usage << " byte(s)" <<std::endl;
}

NS_LEMON_RESOURCE_END