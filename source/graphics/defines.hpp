// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_LEMON_GRAPHICS_BEGIN namespace lemon { namespace graphics {
#define NS_LEMON_GRAPHICS_END } }

NS_LEMON_GRAPHICS_BEGIN

const static size_t kMaxVertexAttributes = 8;
const static size_t kMaxTextures = 8;
const static size_t kMaxUniforms = 32;

struct Backend;
struct GraphicsObject;
struct IndexBuffer;
struct VertexBuffer;
struct Shader;
struct Texture;
struct Texture2D;

NS_LEMON_GRAPHICS_END