// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_FLOW2D_GFX_BEGIN namespace flow2d { namespace graphics {
#define NS_FLOW2D_GFX_END } }

#if defined(PLATFORM_ANDROID)
#define GL_ES_VERSION_2_0
#elif defined(PLATFORM_IOS)
#define GL_ES_VERSION_2_0
#endif

NS_FLOW2D_GFX_BEGIN

const static size_t kMaxVertexAttributes = 8;
const static size_t kMaxTextures = 8;

struct Device;
struct GPUObject;
struct IndexBuffer;
struct VertexBuffer;
struct Shader;
struct Texture;
struct Texture2D;

NS_FLOW2D_GFX_END