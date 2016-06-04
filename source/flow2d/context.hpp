// @date 2016/06/04
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/forward.hpp>

NS_FLOW2D_BEGIN

struct Context
{
    static Context* create();

    EntityManager&          world();
    const EntityManager&    world() const;

    EventManager&           dispatcher();
    const EventManager&     dispatcher() const;

    SystemManager&          system();
    const SystemManager&    system() const;

    void update(float);

public:
    bool initialize();

    std::unique_ptr<EntityManager>  m_world;
    std::unique_ptr<EventManager>   m_dispatcher;
    std::unique_ptr<SystemManager>  m_system;
};

NS_FLOW2D_END