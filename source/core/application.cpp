// @date 2016/08/01
// @author Mao Jingkai(oammix@gmail.com)

#include <core/application.hpp>
#include <SDL2/SDL.h>

NS_FLOW2D_CORE_BEGIN

bool Application::initialize()
{
    // initialize SDL here.
    if( 0 != SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) )
    {
        LOGW("failed to initialize SDL2, %s.", SDL_GetError());
        return false;
    }

    return true;
}

void Application::dispose()
{
    // shutdown SDL now. application should be the last SDL-using subsystem to be destroyed
    SDL_Quit();
}

Platform Application::get_platform() const
{
#if defined(PLATFORM_IOS)
    return Platform::IOS;
#elif defined(ANDROID)
    return Platform::ANDROID;
#else
    return Platform::DESKTOP;
#endif
}

NS_FLOW2D_CORE_END