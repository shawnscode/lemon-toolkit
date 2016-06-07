// @date 2016/06/04
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/context.hpp>

#include <flow2d/core/event.hpp>
#include <flow2d/core/entity.hpp>
#include <flow2d/core/system.hpp>

NS_FLOW2D_BEGIN

Context* Context::create()
{
    auto ctx = new (std::nothrow) Context();
    if( ctx && ctx->initialize() ) return ctx;
    if( ctx ) delete ctx;
    return nullptr;
}

bool Context::initialize()
{
    m_dispatcher.reset(new (std::nothrow) EventManager());
    if( !m_dispatcher ) return false;

    m_world.reset(new (std::nothrow) EntityManager(*m_dispatcher));
    if( !m_world ) return false;

    m_system.reset(new (std::nothrow) SystemManager(*m_world, *m_dispatcher));
    if( !m_system ) return false;

    return true;
}

void Context::update(float dt)
{
    m_system->update(dt);
}

NS_FLOW2D_END