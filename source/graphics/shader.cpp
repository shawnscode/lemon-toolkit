// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/shader.hpp>
#include <graphics/opengl.hpp>

NS_FLOW2D_GFX_BEGIN

static const unsigned GL_ELEMENT_FORMAT[] =
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_FIXED,
    GL_FLOAT
};

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

bool Shader::initialize()
{
    ENSURE( !_device.is_device_lost() );

    if( _fragment_shader.empty() || _vertex_shader.empty() )
        return false;

    if( !_handle )
        _handle = glCreateProgram();
    
    if( !_handle )
    {
        LOGW("failed to create program name.");
        return false;
    }

    auto vs = compile(GL_VERTEX_SHADER, _vertex_shader.c_str());
    if( vs == 0 )
        return false;

    auto fs = compile(GL_FRAGMENT_SHADER, _fragment_shader.c_str());
    if( fs == 0 )
        return false;

    glAttachShader(_handle, vs);
    glAttachShader(_handle, fs);
    glLinkProgram(_handle);

    glDetachShader(_handle, fs);
    glDetachShader(_handle, vs);
    glDeleteShader(fs);
    glDeleteShader(vs);

    GLint status;
    glGetProgramiv(_handle, GL_LINK_STATUS, &status);
    if( status == 0 )
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(_handle, 1024, &len, buf);
        printf("link failed:%s\n", buf);
        return false;
    }

#ifndef GL_ES_VERSION_2_0
    glGenVertexArrays(1, &_vao);
#endif

    CHECK_GL_ERROR();
    return true;
};

void Shader::dispose()
{
    if( _device.is_device_lost() )
        _handle = 0;

    if( _handle != 0 )
    {
        if( _device.get_bound_shader() == _handle )
            _device.set_shader(0);
        glDeleteProgram(_handle);
        _handle = 0;
    }
}

void Shader::bind()
{
    if( !_handle || _device.is_device_lost() )
        return;

    _device.set_shader(_handle);

#ifndef GL_ES_VERSION_2_0
    glBindVertexArray(_vao);
#endif

    unsigned vbo = 0;
    for( unsigned i = 0; i < _attributes.size(); i++ )
    {
        auto& attribute = _attributes[i];
        if( attribute.vbo != vbo )
        {
            _device.set_vertex_buffer(attribute.vbo);
            vbo = attribute.vbo;
        }

        if( vbo != 0 )
        {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i,
                attribute.size,
                GL_ELEMENT_FORMAT[to_value(attribute.format)],
                attribute.normalized,
                attribute.stride,
                (uint8_t*)0+attribute.offset);
        }
       else
            glDisableVertexAttribArray(i);
    }
}

void Shader::set_vertex_attribute(const char* name,
    unsigned size,
    ElementFormat format,
    bool normalized,
    unsigned stride,
    unsigned offset)
{
    ASSERT( name, "invalid attribute name." );

    if( !_handle || _device.is_device_lost() )
    {
        LOGW("trying to set vertex attribute while device is lost.");
        return;
    }

    auto vbo = _device.get_bound_vertex_buffer();
    if( !vbo )
    {
        LOGW("trying to set vertex attribute without a bound VBO.");
        return;
    }

    for( unsigned i = 0; i < _attributes.size(); i++ )
    {
        if( _attributes[i].name == name )
        {
            _attributes[i].vbo = vbo;
            _attributes[i].size = size;
            _attributes[i].format = format;
            _attributes[i].normalized = normalized;
            _attributes[i].stride = stride;
            _attributes[i].offset = offset;
            return;
        }
    }

    auto location = glGetAttribLocation(_handle, name);
    if( location == - 1 )
    {
        LOGW("failed to locate attribute %s.", name);
        return;
    }

    if( _attributes.size() <= location )
        _attributes.resize(location+1);

    _attributes[location].name = math::StringHash(name);
    _attributes[location].vbo = vbo;
    _attributes[location].size = size;
    _attributes[location].format = format;
    _attributes[location].normalized = normalized;
    _attributes[location].stride = stride;
    _attributes[location].offset = offset;
    CHECK_GL_ERROR();
}

int32_t Shader::get_uniform_location(const char* name)
{
    if( !_handle || _device.is_device_lost() )
    {
        LOGW("trying to set uniform while device is lost.");
        return -1;
    }

    for( auto& pair : _locations )
    {
        if( pair.first == name )
            return pair.second;
    }

    auto location = glGetUniformLocation(_handle, name);
    if( location == -1 )
        LOGW("failed to locate uniform %s.", name);

    _locations.push_back(std::make_pair(math::StringHash(name), location));
    return location;
}

void Shader::set_uniform1f(const char* name, const math::Vector<1, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniform1f(pos, value[0]);
    CHECK_GL_ERROR();
}

void Shader::set_uniform2f(const char* name, const math::Vector<2, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniform2f(pos, value[0], value[1]);
    CHECK_GL_ERROR();
}

void Shader::set_uniform3f(const char* name, const math::Vector<3, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniform3f(pos, value[0], value[1], value[2]);
    CHECK_GL_ERROR();
}

void Shader::set_uniform4f(const char* name, const math::Vector<4, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniform4f(pos, value[0], value[1], value[2], value[3]);
    CHECK_GL_ERROR();
}

void Shader::set_uniform2fm(const char* name, const math::Matrix<2, 2, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniformMatrix2fv(pos, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
}

void Shader::set_uniform3fm(const char* name, const math::Matrix<3, 3, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniformMatrix3fv(pos, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
}

void Shader::set_uniform4fm(const char* name, const math::Matrix<4, 4, float>& value)
{
    auto pos = get_uniform_location(name);
    if( pos == -1 )
        return;

    _device.set_shader(_handle);
    glUniformMatrix4fv(pos, 1, GL_TRUE, (float*)&value);
    CHECK_GL_ERROR();
}


NS_FLOW2D_GFX_END