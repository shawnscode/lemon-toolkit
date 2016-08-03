// @date 2016/08/01
// @author Mao Jingkai(oammix@gmail.com)

#include <core/defines.hpp>
#include <core/context.hpp>

NS_FLOW2D_CORE_BEGIN

// application subsystem. manages the executing environments, should be the first
// subsystem to be created.
struct Application : Subsystem
{
    SUBSYSTEM("Application")

    Application(Context& c) : Subsystem(c) {}
    virtual ~Application() {}

    bool initialize() override;
    void dispose() override;

    Platform get_platform() const;
};

NS_FLOW2D_CORE_END