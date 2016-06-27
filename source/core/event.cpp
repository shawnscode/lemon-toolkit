// @date 2016/05/24
// @author Mao Jingkai(oammix@gmail.com)

#include <core/event.hpp>

NS_FLOW2D_BEGIN

Event::Type Event::s_type_counter = 0;

EventDispatcher::~EventDispatcher()
{
    if( m_ring )
    {
        while( m_ring->next != m_ring ) unlink(m_ring->next);
        unlink(m_ring);
        m_ring = nullptr;
    }
}

EventManager::~EventManager()
{
    for( auto cursor : m_dispatchers )
        if( cursor ) delete cursor;
}

NS_FLOW2D_END