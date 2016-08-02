// @date 2016/07/31
// @author Mao Jingkai(oammix@gmail.com)
// do NOT include this file in headers, make sure the implementation details
// is invisble to user.

#pragma once

#include <defines.hpp>

#if defined(PLATFORM_ANDROID)
#define GL_ES_VERSION_2_0
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(PLATFORM_IOS)
#define GL_ES_VERSION_2_0
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif defined(PLATFORM_DESKTOP)
#include <GL/glew.h>
#else
#error NOT SUPPORT PLATFORM
#endif

#include <SDL2/SDL.h>