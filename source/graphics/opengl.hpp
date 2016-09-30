// @date 2016/07/31
// @author Mao Jingkai(oammix@gmail.com)
// do NOT include this file in headers, make sure the implementation details
// is invisble to user.

#pragma once

#include <defines.hpp>

NS_LEMON_GRAPHICS_BEGIN

#if defined(PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define GL_ES_VERSION_2_0
#elif defined(PLATFORM_IOS)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define GL_ES_VERSION_2_0
#else
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>

extern void check_device_error(const char* file, unsigned line);

#define CHECK_GL_ERROR() check_device_error(__FILE__, __LINE__);

NS_LEMON_GRAPHICS_END