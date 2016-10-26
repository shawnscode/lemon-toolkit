// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/program.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

GLuint compile(GLenum type, const char* source)
{
    GLint status;
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char buf[1024];
        GLint len;
        glGetShaderInfoLog(shader, 1024, &len, buf);

        LOGW("failed to compile, %s. source:\n %s\n", buf, source);
        glDeleteShader(shader);
        return 0;
    }

    CHECK_GL_ERROR();
    return shader;
}

bool ProgramGL::initialize(const char* vsraw, const char* psraw)
{
    if( vsraw == nullptr || psraw == nullptr )
    {
        LOGW("failed to restore shader without vertex/fragment shader.");
        return false;
    }

    dispose();

    _object = glCreateProgram();
    if( _object == 0 )
    {
        LOGW("failed to create program object.");
        return false;
    }

    auto vs = compile(GL_VERTEX_SHADER, vsraw);
    if( vs == 0 )
        return false;

    auto fs = compile(GL_FRAGMENT_SHADER, psraw);
    if( fs == 0 )
        return false;

    glAttachShader(_object, vs);
    glAttachShader(_object, fs);
    glLinkProgram(_object);

    glDetachShader(_object, fs);
    glDetachShader(_object, vs);
    glDeleteShader(fs);
    glDeleteShader(vs);

    GLint status;
    glGetProgramiv(_object, GL_LINK_STATUS, &status);
    if( status == 0 )
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(_object, 1024, &len, buf);
        LOGW("failed to link program: %s\n", buf);
        return false;
    }

    _vertex_shader = vsraw;
    _fragment_shader = psraw;
    CHECK_GL_ERROR();
    return true;
}

void ProgramGL::dispose()
{
    if( _object != 0 )
        glDeleteProgram(_object);

    _object = 0;
    _uniforms.clear();
    for( uint8_t i = 0; i < VertexAttribute::kVertexAttributeCount; i++ )
        _attributes[i] = -1;
}

GLint ProgramGL::get_uniform_location(const char* name, bool warning) const
{
    if( _object == 0 )
    {
        LOGW("failed to get uniform location without a decent handle.");
        return -1;
    }

    auto hash = math::StringHash(name);
    auto found = _uniforms.find(hash);
    if( found != _uniforms.end() )
        return found->second;

    auto location = glGetUniformLocation(_object, name);
    if( location == -1 && warning )
        LOGW("failed to locate uniform %s of program %d.", name, _object);

    _uniforms.insert(std::make_pair(hash, location));
    return location;
}

GLint ProgramGL::get_attribute_location(const char* name)
{
    if( _object == 0 )
    {
        LOGW("failed to get attribute location without a decent handle.");
        return -1;
    }

    auto location = glGetAttribLocation(_object, name);
    if( location == -1 )
        LOGW("failed to localte attribute %s of program %d.", name, _object);

    return location;
}

GLint ProgramGL::get_attribute_location(VertexAttribute::Enum va)
{
    return _attributes[value(va)];
}

bool ProgramGL::set_attribute_name(VertexAttribute::Enum va, const char* name)
{
    auto location = get_attribute_location(name);
    _attributes[value(va)] = location;
    return location != -1;
}

bool ProgramGL::has_uniform(const char* name) const
{
    return get_uniform_location(name, false) != -1;
}

NS_LEMON_GRAPHICS_END
