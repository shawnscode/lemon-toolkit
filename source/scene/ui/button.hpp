// @date 2016/07/04
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <scene/ui/element.hpp>

NS_FLOW2D_BEGIN

struct UIButton : public IViewController
{
    enum class state : uint8_t
    {
        NORMAL          = 0,
        HIGHTLIGHTED    = 1,
        PRESSED         = 2,
        DISABLE         = 3,
        MAX             = 4
    };

    void draw(UIElement&, Canvas&) override;
    void update(float) override;

    void set_fade_duration(float);
    void set_state_color(state, const Color&);
    Vector2f get_prefered_size() const override;

protected:
    state _current_state    = state::NORMAL;
    state _last_state       = state::NORMAL;

    Color _colors[(size_t)state::MAX];

    float _fade_duration    = 0.25f;
    float _fade_ticker      = 0.f;
};

NS_FLOW2D_END