// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/system.hpp>
#include <math/matrix.hpp>
#include <ui/widget.hpp>
#include <ui/listener.hpp>

NS_FLOW2D_UI_BEGIN

enum class ResolutionResolveMode : uint8_t
{
    // according to the width and height of screen and design resolution to determine
    // the scale factor, choose the smaller value of factor as the scale factor.
    // this can make sure that all the design area can display on screen,
    // but may leave some area black on screen
    SCALE_SHOW_ALL,

    // according to the width and height of screen and design resolution to determine
    // the scale factor, choose the larger one as the scale factor.
    // This can make sure that one axis can always fully display on screen,
    // but another may scale out of the screen
    SCALE_NO_BORDER,

    // set the width ratio of the screen and design resolution as scale factor in X axis,
    // set the height ratio of the screen and design resolution as scale factor in Y axis.
    // this can make sure the design area cover the whole screen,
    // but the picture maybe stretched
    SCALE_EXACT_FIT,

    // keep the width of the design resolution,
    // refine the width of the design resolution according to the screen
    REFINE_FIXED_WIDTH,

    // keep the height of the design resolution,
    // refine the height of the design resolution according to the screen
    REFINE_FIXED_HEIGHT,

    // refine the width/height of the design resolution according to the screen
    REFINE_ALL
};

// struct CanvasDirector : public Component<1>

struct CanvasDirector : public ComponentWithEnvironment<1>
{
    CanvasDirector();

    void set_resolve_mode(ResolutionResolveMode);
    void set_screen_size(const Vector2f&);
    void set_design_size(const Vector2f&);

    Matrix3f get_ortho() const;
    Vector2f get_resolved_size() const;

    void resize(Transform&);
    bool invoke();

protected:
    void resolve();
    void resize_recursive(Transform&, const Rect2f&);

    Matrix3f                _ortho;
    Vector2f                _resolved_size  = {960.f, 640.f};

    Vector2f                _design_size    = {960.f, 640.f};
    Vector2f                _screen_size    = {960.f, 640.f};
    ResolutionResolveMode   _resolve_mode   = ResolutionResolveMode::REFINE_ALL;
};

struct CanvasSystem : public SystemWithEntities<CanvasDirector>
{
    CanvasSystem(EntityManager& world) : SystemWithEntities<CanvasDirector>(world) {}

    void on_spawn(SystemManager&) override;
    void on_dispose(SystemManager&) override;
    void receive(const EvtInputMouse&);
    void receive(const EvtInputMousePosition&);

    void set_screen_size(const Vector2f&);
    void update(float);
    void draw();

protected:
    struct MouseState
    {
        ButtonAction action;
        Vector2f     current;   // current mouse position
        Vector2f     last;      // mouse position of last frame
        Vector2f     start;
        float        pressed;   // pressed timer
        Entity       object;    // focused object
    };

    Vector2f screen_to_design(const Vector2f&);
    void set_mouse_focus(MouseButton, Entity);

    MouseState              _mouse_states[kMaxMouseButton];
    std::unique_ptr<Canvas> _canvas;
    Vector2f                _screen_size;
};

NS_FLOW2D_UI_END