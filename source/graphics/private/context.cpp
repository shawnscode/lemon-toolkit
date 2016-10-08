#include <graphics/private/context.hpp>

NS_LEMON_GRAPHICS_BEGIN

static const char* to_string(GLenum error)
{
    switch(error) {
        case GL_INVALID_OPERATION:
            return "INVALID_OPERATION";
        case GL_INVALID_ENUM:
            return "INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "INVALID_VALUE";
        case GL_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "INVALID_FRAMEBUFFER_OPERATION";
    }
    return "UNDEFINED";
}

void check_device_error(const char* file, unsigned line)
{
    GLenum error = glGetError();
    if( error != GL_NO_ERROR && error != GL_INVALID_ENUM )
        FATAL("GL_%s", to_string(error));
}

NS_LEMON_GRAPHICS_END