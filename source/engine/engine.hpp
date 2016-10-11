// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/subsystem.hpp>

#include <chrono>
#include <vector>

NS_LEMON_BEGIN

// lemon engine, creates the other subsystems
struct Engine : public core::Subsystem
{
    using timepoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::high_resolution_clock::duration;

    SUBSYSTEM("Engine");
    Engine() {}
    virtual ~Engine() {}

    bool initialize() override;
    void dispose() override;

    // perform on frame with specified fps, this will call update/render internally
    void run_one_frame();
    // send frame update events
    void update(duration);
    // render after frame update
    void render();
    // poll window messages and dispatch it internally
    void process_message();

    // set minimum frames per second. if fps goes lower than this, time will appear to slow.
    void set_min_fps(unsigned);
    // set maximum frames per second. the engine will sleep if fps is higher than this.
    void set_max_fps(unsigned);
    // set maximum frames per second when the application does not have input focus
    void set_max_inactive_fps(unsigned);
    // set how many frames to average for timestep smoothing.
    void set_time_smoothing_step(unsigned);
    // set whether to pause update when minimized
    void set_pause_minimized(bool);
    // return if engine is exiting
    bool is_running() const { return _running; }

protected:
    // minimum/maximum frames per second
    unsigned _min_fps, _max_fps, _max_inactive_fps;
    // previous timesteps for smoothing in seconds
    std::vector<duration> _previous_timesteps;
    // next frame timestep in seconds
    duration _timestep;
    // how many frames to average for the smoothed timestep
    unsigned _smoothing_step;
    // pause when minimized
    bool _pause_minimized;
    // frame update timer
    timepoint _last_frame_timepoint;
    // exiting flag
    bool _running;
};

// application-wide logic update event
struct EvtUpdate
{
    EvtUpdate(Engine::duration timestep) : timestep(timestep) {}
    Engine::duration timestep;
};

// application-wide logic post-update event
struct EvtPostUpdate
{
    EvtPostUpdate(Engine::duration timestep) : timestep(timestep) {}
    Engine::duration timestep;
};

// render update event
struct EvtRenderUpdate
{
    EvtRenderUpdate(Engine::duration timestep) : timestep(timestep) {}
    Engine::duration timestep;
};

// post-render update event
struct EvtPostRenderUpdate
{
    EvtPostRenderUpdate(Engine::duration timestep) : timestep(timestep) {}
    Engine::duration timestep;
};

// frame render event
struct EvtRender {};

NS_LEMON_END