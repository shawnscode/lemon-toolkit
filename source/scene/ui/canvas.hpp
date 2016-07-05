// @date 2016/07/05
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/entity.hpp>
#include <math/matrix.hpp>
#include <scene/ui/element.hpp>

NS_FLOW2D_BEGIN

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

struct CanvasScaler : public Component<kUiComponentsChunkSize>
{
    CanvasScaler();

    void set_resolve_mode(ResolutionResolveMode);
    void set_screen_size(const Vector2f&);
    void set_design_size(const Vector2f&);
    Matrix3f get_ortho() const;

protected:
    Matrix3f resolve() const;

    Matrix3f                _ortho;
    Vector2f                _design_size    = {960.f, 640.f};
    Vector2f                _screen_size    = {960.f, 640.f};
    ResolutionResolveMode   _resolve_mode   = ResolutionResolveMode::REFINE_ALL;
};

struct CanvasSystem
{
    CanvasSystem(EntityManager&);
    ~CanvasSystem();

    void receive(const EvtComponentAdded<CanvasScaler>&);
    void receive(const EvtComponentRemoved<CanvasScaler>&);

    void set_screen_size(const Vector2f&);
    void update(float);
    void draw();

protected:
    std::unordered_map<Entity, CanvasScaler*>   _scalers;
    std::unique_ptr<Canvas>                     _canvas;
    EntityManager&                              _world;
    Vector2f                                    _screen_size;
};

NS_FLOW2D_END