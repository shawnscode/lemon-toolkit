// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#include <engine/input.hpp>

#include <SDL2/SDL.h>

NS_FLOW2D_BEGIN

bool Input::initialize()
{
    subscribe<Input, EvtBeginFrame>();
    return true;
}

void Input::dispose()
{
    unsubscribe<Input, EvtBeginFrame>();
}

void Input::receive(const EvtBeginFrame&)
{

    SDL_Event event;
    // while( SDL_PollEvent(&event) )
    // {
    //     switch( event.type )
    //     {
    //         case SDL_KEYDOWN:

    //     }
    // }
}

math::Vector2f Input::get_mouse_position() const
{
    return {0.f, 0.f};
}

math::Vector2f Input::get_mouse_delta() const
{
    return {0.f, 0.f};
}

bool Input::has_focus() const
{
    return true;
}

bool Input::is_minimized() const
{
    return false;
}

NS_FLOW2D_END