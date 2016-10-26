// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#include <engine/engine.hpp>
#include <engine/input.hpp>
#include <engine/arguments.hpp>
#include <graphics/window.hpp>
#include <graphics/renderer.hpp>
#include <resource/resource.hpp>
#include <resource/archives.hpp>
#include <resource/filesystem.hpp>
#include <core/event.hpp>
#include <core/ecs.hpp>
#include <core/task.hpp>

#include <scene/scene.hpp>

#include <SDL2/SDL.h>

NS_LEMON_BEGIN

bool Engine::initialize()
{
    // initialize SDL here.
    if( 0 != SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) )
    {
        LOGW("failed to initialize SDL2, %s.", SDL_GetError());
        return false;
    }

    core::add_subsystem<core::EventSystem>();
    core::add_subsystem<core::EntityComponentSystem>();
    core::add_subsystem<core::JobSystem>();
    core::add_subsystem<graphics::WindowDevice>();
    core::add_subsystem<graphics::Renderer>();
    core::add_subsystem<res::ArchiveCollection>();
    core::add_subsystem<res::ResourceCache>();
    core::add_subsystem<Input>();
    core::add_subsystem<Scene>();

    auto arguments = core::get_subsystem<Arguments>();

    // create window device and restore OpenGL state internally
    auto width = arguments->fetch("/Graphics/WindowSize/0", 128).GetInt();
    auto height = arguments->fetch("/Graphics/WindowSize/1", 128).GetInt();
    auto multisamples = arguments->fetch("/Graphics/Multisamples", 1).GetInt();

    auto device = core::get_subsystem<graphics::WindowDevice>();
    if( !device->open(width, height, multisamples, graphics::WindowOption::RESIZABLE) )
        return false;

    // initialize filesystem and configurated archives
    if( auto pwd = arguments->fetch("/WorkingDirectory") )
        fs::set_current_directory( arguments->get_path() / fs::Path(pwd->GetString()) );

    if( auto searches = arguments->fetch("/Resource/SearchPaths") )
    {
        auto collection = core::get_subsystem<res::ArchiveCollection>();
        for( auto& path : searches->GetArray() )
            if( !collection->add_search_path(arguments->get_path() / path.GetString()) )
                return false;
    }

    // initialize resource 
    auto cache = core::get_subsystem<res::ResourceCache>();
    auto memory_threshold = arguments->fetch("/Resource/CacheMemoryThresholdInMB", 64).GetInt();
    auto video_memory_threshold = arguments->fetch("/Resource/CacheVideoMemoryThresholdInMB", 64).GetInt();
    memory_threshold *= (1024 * 1024);
    video_memory_threshold *= (1024 * 1024);
    cache->set_threshold(memory_threshold, video_memory_threshold);

    // fire engine
    _running = true;
    _timestep.zero();
    _last_frame_timepoint = clock::now();
    _launch_timepoint = clock::now();

    _min_fps = arguments->fetch("/Engine/MinFPS", 0).GetInt();
    _max_fps = arguments->fetch("/Engine/MaxFPS", 0).GetInt();
    _max_inactive_fps = arguments->fetch("/Engine/MaxInactiveFPS", 0).GetInt();
    _smoothing_step = arguments->fetch("/Engine/TimeSmoothingStep", 0).GetInt();
    return true;
}

void Engine::dispose()
{
    // shutdown SDL now
    SDL_Quit();
}

void Engine::run_one_frame()
{
    auto device = core::get_subsystem<graphics::WindowDevice>();
    auto input = core::get_subsystem<Input>();

    input->begin_frame();
    process_message();

    if( !_running )
        return;

    // if pause when minimized-mode is in use, stop update
    if( !_pause_minimized || !device->is_minimized() )
        update(_timestep);

    input->end_frame();

    // perform waiting loop if maximum fps set
    auto max_fps = _max_fps;
    if( (device->get_window_flags() & SDL_WINDOW_INPUT_FOCUS) && max_fps > 0 )
        max_fps = std::min(_max_inactive_fps, max_fps);

    if( max_fps > 0 )
    {
        duration target_duration = std::chrono::milliseconds(1000) / max_fps;
        duration eplased = clock::now() - _last_frame_timepoint;
        for( ;; )
        {
            eplased = clock::now() - _last_frame_timepoint;
            if( eplased > target_duration )
                break;

            if( target_duration - eplased > duration(1) )
                std::this_thread::sleep_for((target_duration - eplased)-duration(1));
        }
    }

    duration eplased = clock::now() - _last_frame_timepoint;
    _last_frame_timepoint = clock::now();

    // if fps lower than minimum, clamp eplased time
    if( _min_fps > 0 )
    {
        duration target_duration = std::chrono::milliseconds(1000) / _min_fps;
        if( eplased > target_duration )
            eplased = target_duration;
    }

    // perform timestep smoothing
    if( _smoothing_step > 0 )
    {
        _timestep.zero();
        _previous_timesteps.push_back(eplased);
        if( _previous_timesteps.size() > _smoothing_step )
        {
            auto begin = _previous_timesteps.begin();
            _previous_timesteps.erase(begin, begin+_previous_timesteps.size()-_smoothing_step);
            for( auto step : _previous_timesteps )
                _timestep += step;
            _timestep /= _previous_timesteps.size();
        }
        else
            _timestep = _previous_timesteps.back();
    }
    else
    {
        _timestep = eplased;
    }
}

void Engine::update(duration dt)
{
    auto renderer = core::get_subsystem<graphics::Renderer>();
    auto event = core::get_subsystem<core::EventSystem>();

    event->emit<EvtUpdate>(dt);
    event->emit<EvtPostUpdate>(dt);

    if( !renderer->is_device_lost() )
    {
        event->emit<EvtRenderUpdate>(dt);
        if( renderer->begin_frame() )
        {
            event->emit<EvtRender>();
            renderer->end_frame();
        }
        event->emit<EvtPostRenderUpdate>(dt);
    }
}

void Engine::process_message()
{
    auto input = core::get_subsystem<Input>();
    auto device = core::get_subsystem<graphics::WindowDevice>();

    SDL_Event event;
    while( SDL_PollEvent( &event ) )
    {
        if( event.type == SDL_QUIT )
        {
            _running = false;
            return;
        }

        input->process_message(&event);
        device->process_message(&event);
    }
}

void Engine::set_min_fps(unsigned fps)
{
    _min_fps = std::max(fps, (unsigned)0);
}

void Engine::set_max_fps(unsigned fps)
{
    _max_fps = std::max(fps, (unsigned)0);
}

void Engine::set_max_inactive_fps(unsigned fps)
{
    _max_inactive_fps = std::max(fps, (unsigned)0);
}

void Engine::set_time_smoothing_step(unsigned step)
{
    _smoothing_step = step;
}

void Engine::set_pause_minimized(bool enable)
{
    _pause_minimized = enable;
}

Engine::duration Engine::get_time_since_launch() const
{
    return clock::now() - _launch_timepoint;
}

unsigned Engine::get_fps() const
{
    return 1000 / (_timestep / std::chrono::milliseconds(1000));
}

NS_LEMON_END
