// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#include <engine/application.hpp>
#include <engine/engine.hpp>
#include <core/instance.hpp>

NS_LEMON_BEGIN

Application::Application() : _exitcode(0)
{}

int Application::run()
{
    setup();
    if( _exitcode != 0 )
        return _exitcode;

    core::initialize(0);
    auto engine = core::add_subsystem<Engine>();

    start();
    if( _exitcode != 0 )
        return _exitcode;

    while( engine->is_running() )
        engine->run_one_frame();

    stop();

    core::dispose();
    return _exitcode;
}

void Application::terminate_with_error(const std::string& message)
{
    LOGE(message.c_str());
    _exitcode = -1;
}

NS_LEMON_END