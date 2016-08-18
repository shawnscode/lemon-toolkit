// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <engine/engine.hpp>
#include <math/vector.hpp>

NS_FLOW2D_BEGIN

// input subsystem. convert operating system window messages to input state
// and internal events.
struct Input : public core::Subsystem
{
    SUBSYSTEM("Input");
    Input(core::Context& c) : Subsystem(c) {}
    virtual ~Input() {}

    bool initialize() override;
    void dispose() override;

    // poll for window messages
    void receive(const EvtBeginFrame&);

    // return mouse position within window. should only be used with a visible mouse cursor
    math::Vector2f get_mouse_position() const;
    // return mouse movement since last frame
    math::Vector2f get_mouse_delta() const;

    // return whether application window has input focus
    bool has_focus() const;
    // return whether application window is minimized
    bool is_minimized() const;

    bool get_key_press(const std::string&) { return false; }
};

NS_FLOW2D_END