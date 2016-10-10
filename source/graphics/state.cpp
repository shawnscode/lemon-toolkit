// @date 2016/08/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/state.hpp>

NS_LEMON_GRAPHICS_BEGIN

RenderState::RenderState()
{
    reset();
}

void RenderState::reset()
{
    primitive = PrimitiveType::TRIANGLES;

    cull.enable   = false;
    cull.face     = CullFace::BACK;
    cull.winding  = FrontFaceOrder::COUNTERCLOCKWISE;

    scissor.enable = false;
    scissor.area   = math::Rect2i { { 0, 0 }, { 1, 1 } };

    depth.enable = false;
    depth.compare = CompareEquation::NEVER;
    depth_write.enable = false;
    depth_write.bias_slope_scaled = 0.f;
    depth_write.bias_constant = 0.f;

    blend.enable = false;
    blend.equation = BlendEquation::ADD;
    blend.source_factor = BlendFactor::SRC_ALPHA;
    blend.destination_factor = BlendFactor::INV_SRC_ALPHA;
    color_write = ColorMask::ALL;

    stencil.enable = false;
    stencil.reference = 0;
    stencil.mask = 0;
    stencil.compare = CompareEquation::NEVER;

    stencil_write.mask = ~((unsigned)0);
    stencil_write.sfail = StencilWriteEquation::KEEP;
    stencil_write.dpfail = StencilWriteEquation::KEEP;
    stencil_write.dppass = StencilWriteEquation::KEEP;
}

NS_LEMON_GRAPHICS_END