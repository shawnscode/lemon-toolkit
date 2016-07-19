// @date 2016/07/04
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <ui/widget.hpp>
#include <ui/listener.hpp>

NS_FLOW2D_UI_BEGIN

struct Button : public View
{
    enum class state : uint8_t
    {
        NORMAL          = 0,
        HIGHTLIGHTED    = 1,
        PRESSED         = 2,
        DISABLE         = 3,
    };

    void on_spawn(EntityManager&, Entity) override;
    void on_dispose(EntityManager&, Entity) override;

    void on_draw(Canvas&, const Rect2f&) override;
    void on_update(float) override;

    void set_state(state);
    void set_fade_duration(float);
    void set_state_color(state, const Color&);

    void receive(Transform&, EvtMouseDown&);
    void receive(Transform&, EvtMouseClick&);

protected:
    state _current_state    = state::NORMAL;
    state _last_state       = state::NORMAL;

    Color _colors[4]        = { {1.0f, 1.0f, 1.0f}, {1.25f, 1.25f, 1.25f}, {0.59, 0.59, 0.59} };
    float _fade_duration    = 0.25f;
    float _fade_ticker      = 0.f;
};

NS_FLOW2D_UI_END