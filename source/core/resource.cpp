#include <core/resource.hpp>
#include <core/archive.hpp>

NS_FLOW2D_BEGIN

ResourceId::ResourceId()
{
    index = std::numeric_limits<uint32_t>::max();
    version = 0;
    type = 0;
}

ResourceId::ResourceId(uint32_t index, uint32_t version, uint32_t type)
{
    this->index = index;
    this->version = version & 0xFFF;
    this->type = type & 0xF;
}

Resource::RuntimeTypeId Resource::s_rti = 0;

ResourceCacheManager::ResourceCacheManager(ArchiveManager& archive, size_t threshold)
: m_archives(archive), m_memory_usage(0), m_cache_threshold(threshold)
{}

ResourceCacheManager::~ResourceCacheManager()
{
    for( auto i=0; i<m_resources.size(); i++ )
        delete m_resources[i];
}

size_t ResourceCacheManager::get_memory_usage() const
{
    return m_memory_usage;
}

void ResourceCacheManager::touch(ResourceId id)
{
    auto cursor = std::find(m_lru.begin(), m_lru.end(), id);
    if( cursor != m_lru.end() )
    {
        m_lru.push_back(*cursor);
        m_lru.erase(cursor);
    }
}

ResourceId ResourceCacheManager::try_insert(Resource* resource, const char* name, uint32_t size)
{
    auto stream = m_archives.open({name});
    if( !stream || !resource->load_from_file(std::move(stream)) )
    {
        delete resource;
        return ResourceId();
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

    auto id = ResourceId(index, version, size );
    m_identifiers[name] = id;

    m_resources[index] = resource;
    m_refcounts[index] = 1;
    m_memory_usage += resource->get_memory_usage();

    if( m_memory_usage > m_cache_threshold )
        try_make_room( (m_memory_usage - m_cache_threshold)*1.25f );

    m_lru.push_back(id);
    return id;
}

void ResourceCacheManager::try_make_room(size_t size)
{
    if( m_lru.size() == 0 || size <= 0 )
        return;

    size_t removed = 0;
    for( auto cursor = m_lru.begin(); cursor != m_lru.end(); )
    {
        auto index = (*cursor).index;
        if( m_refcounts[index] != 1 )
        {
            cursor ++;
        }
        else
        {
            removed += m_resources[index]->get_memory_usage();
            m_lru.erase( cursor ++ );

            delete m_resources[index];
            m_resources[index] = nullptr;

            m_versions[index] ++;
            m_freeslots.push_back(index);

            if( removed >= size )
                break;
        }
    }

    m_memory_usage -= removed;
}

NS_FLOW2D_END