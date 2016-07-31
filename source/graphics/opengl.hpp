// @date 2016/07/31
// @author Mao Jingkai(oammix@gmail.com)
// do NOT include this file in headers, make sure the implementation details
// is invisble to user.

#pragma once

#include <defines.hpp>

#if defined(PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(PLATFORM_IOS)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>