// @date 2016/08/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/render_state.hpp>

NS_LEMON_GRAPHICS_BEGIN

RenderState::RenderState()
{
    reset_render_state(*this);
}

void reset_render_state(RenderState& state)
{
    state.cull.enable   = false;
    state.cull.face     = CullFace::BACK;
    state.cull.winding  = FrontFaceOrder::COUNTERCLOCKWISE;

    state.scissor.enable = false;
    state.scissor.area   = math::Rect2i { { 0, 0 }, { 1, 1 } };

    state.stencil.enable = false;
    state.stencil.reference = 0;
    state.stencil.mask = 0;
    state.stencil.compare = CompareEquation::NEVER;

    state.stencil_write.mask = ~((unsigned)0);
    state.stencil_write.sfail = StencilWriteEquation::KEEP;
    state.stencil_write.dpfail = StencilWriteEquation::KEEP;
    state.stencil_write.dppass = StencilWriteEquation::KEEP;

    state.depth.enable = false;
    state.depth.compare = CompareEquation::NEVER;

    state.depth_write.enable = false;

    state.blend.enable = false;
    state.blend.equation = BlendEquation::ADD;
    state.blend.source_factor = BlendFactor::SRC_ALPHA;
    state.blend.destination_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;

    state.color_write = ColorMask::ALL;
}

NS_LEMON_GRAPHICS_END