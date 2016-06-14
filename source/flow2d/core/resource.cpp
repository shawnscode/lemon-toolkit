#include <flow2d/core/resource.hpp>

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

size_t ResourceCacheManager::get_memory_usage() const
{
    size_t size = 0;
    for( auto pair : m_identifiers )
        size += m_resources[pair.second.index]->get_memory_usage();
    return size;
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
}

NS_FLOW2D_END