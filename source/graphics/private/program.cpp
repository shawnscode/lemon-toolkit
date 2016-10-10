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
    ENSURE_NOT_RENDER_PHASE;

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
    ENSURE_NOT_RENDER_PHASE;

    if( _object != 0 )
        glDeleteProgram(_object);

    _object = 0;
    _textures.clear();
    _uniforms.clear();
}

void ProgramGL::bind()
{
    if( _object == 0 )
    {
        LOGW("failed to bind program without a decent handle.");
        return;
    }

    glUseProgram(_object);
    unsigned unit = 0;
    for( auto& pair : _textures )
    {
        auto texture = std::static_pointer_cast<TextureGL>(pair.second.second);
        glActiveTexture(GL_TEXTURE0+unit);
        glBindTexture(GL_TEXTURE_2D, texture->get_handle());
        glUniform1i(pair.second.first, unit++);
    }

    CHECK_GL_ERROR();
}

GLint ProgramGL::get_uniform_location(const char* name)
{
    if( _object == 0 )
    {
        LOGW("failed to get uniform localtion without a decent handle.");
        return -1;
    }

    auto hash = math::StringHash(name);
    auto found = _uniforms.find(hash);
    if( found != _uniforms.end() )
        return found->second;

    auto localtion = glGetUniformLocation(_object, name);
    if( localtion == -1 )
        LOGW("failed to locate uniform %s.", name);

    _uniforms.insert(std::make_pair(hash, localtion));
    return localtion;
}

bool ProgramGL::set_uniform_texture(const char* name, std::shared_ptr<Texture> texture)
{
    ENSURE_NOT_RENDER_PHASE;

    auto hash = math::StringHash(name);

    auto found = _textures.find(hash);
    if( found != _textures.end() )
    {
        found->second.second = texture;
        return true;
    }

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    _textures.insert(std::make_pair(hash, std::make_pair(localtion, texture)));
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_1f(const char* name, const math::Vector<1, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniform1f(localtion, value[0]);
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_2f(const char* name, const math::Vector<2, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniform2f(localtion, value[0], value[1]);
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_3f(const char* name, const math::Vector<3, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniform3f(localtion, value[0], value[1], value[2]);
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_4f(const char* name, const math::Vector<4, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniform4f(localtion, value[0], value[1], value[2], value[3]);
    CHECK_GL_ERROR();
    return true;
}

// OpenGL use column-major layout, so we always transpose our matrix
bool ProgramGL::set_uniform_2fm(const char* name, const math::Matrix<2, 2, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniformMatrix2fv(localtion, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_3fm(const char* name, const math::Matrix<3, 3, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniformMatrix3fv(localtion, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
    return true;
}

bool ProgramGL::set_uniform_4fm(const char* name, const math::Matrix<4, 4, float>& value)
{
    ENSURE_NOT_RENDER_PHASE;

    auto localtion = get_uniform_location(name);
    if( localtion == -1 )
        return false;

    glUseProgram(_object);
    glUniformMatrix4fv(localtion, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
    return true;
}

NS_LEMON_GRAPHICS_END
