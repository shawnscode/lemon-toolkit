// // @date 2016/08/03
// // @author Mao Jingkai(oammix@gmail.com)

// #include <graphics/shader.hpp>
// #include <graphics/opengl.hpp>

// NS_LEMON_GRAPHICS_BEGIN

// static const unsigned GL_ELEMENT_FORMAT[] =
// {
//     GL_BYTE,
//     GL_UNSIGNED_BYTE,
//     GL_SHORT,
//     GL_UNSIGNED_SHORT,
//     GL_FIXED,
//     GL_FLOAT
// };

// GLuint compile(GLenum type, const char* source)
// {
//     GLint status;
    
//     GLuint shader = glCreateShader(type);
//     glShaderSource(shader, 1, &source, NULL);
//     glCompileShader(shader);
    
//     glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
//     if (status == GL_FALSE) {
//         char buf[1024];
//         GLint len;
//         glGetShaderInfoLog(shader, 1024, &len, buf);

//         LOGW("failed to compile, %s. source:\n %s\n", buf, source);
//         glDeleteShader(shader);
//         return 0;
//     }

//     CHECK_GL_ERROR();
//     return shader;
// }

// bool Shader::restore(const char* vs, const char* ps)
// {
//     _vertex_shader = vs;
//     _fragment_shader = ps;
//     return restore();
// }

// bool Shader::restore()
// {
//     if( _device.is_device_lost() )
//     {
//         LOGW("trying to restore vertex buffer while device is lost.");
//         return false;
//     }

//     if( _fragment_shader.empty() || _vertex_shader.empty() )
//     {
//         LOGW("failed to restore shader without vertex/fragment shader.");
//         return false;
//     }

//     release();

//     _object = glCreateProgram();
//     if( _object == 0 )
//     {
//         LOGW("failed to create program object.");
//         return false;
//     }

//     auto vs = compile(GL_VERTEX_SHADER, _vertex_shader.c_str());
//     if( vs == 0 )
//         return false;

//     auto fs = compile(GL_FRAGMENT_SHADER, _fragment_shader.c_str());
//     if( fs == 0 )
//         return false;

//     glAttachShader(_object, vs);
//     glAttachShader(_object, fs);
//     glLinkProgram(_object);

//     glDetachShader(_object, fs);
//     glDetachShader(_object, vs);
//     glDeleteShader(fs);
//     glDeleteShader(vs);

//     GLint status;
//     glGetProgramiv(_object, GL_LINK_STATUS, &status);
//     if( status == 0 )
//     {
//         char buf[1024];
//         GLint len;
//         glGetProgramInfoLog(_object, 1024, &len, buf);
//         LOGW("failed to link program: %s\n", buf);
//         return false;
//     }

// #ifndef GL_ES_VERSION_2_0
//     glGenVertexArrays(1, &_vao);
// #endif

//     CHECK_GL_ERROR();
//     return true;
// };

// void Shader::release()
// {
//     if( !_device.is_device_lost() )
//     {
//         if( _object != 0 )
//         {
//             if( _device.get_bound_shader() == _object )
//                 _device.set_shader(0);
//             glDeleteProgram(_object);
//         }

//         if( _vao != 0 )
//             glDeleteVertexArrays(1, &_vao);
//     }

//     _object = 0;
//     _vao = 0;

//     _locations.clear();

//     for( auto& record : _attributes )
//         record.location = 0;

//     for( auto& record : _textures )
//         record.location = 0;
// }

// void Shader::use()
// {
//     if( !_object || _device.is_device_lost() )
//         return;

//     _device.set_shader(_object);

//     unsigned unit = 0;
//     for( auto& record : _textures )
//     {
//         if( record.location == 0 )
//         {
//             auto location = get_uniform_location(record.name.c_str());
//             if( location == -1 )
//                 continue;
//             record.location = location;
//             glUniform1i(location, unit);
//         }

//         _device.set_texture(unit++,
//             record.texture->get_target(),
//             record.texture->get_graphic_object());
//     }

// #ifndef GL_ES_VERSION_2_0
//     glBindVertexArray(_vao);
// #endif

//     unsigned vbo = 0;
//     for( auto& record : _attributes )
//     {
//         if( vbo != record.vb->get_graphic_object() )
//         {
//             vbo = record.vb->get_graphic_object();
//             _device.set_vertex_buffer(vbo);
//         }

//         if( record.location == 0 )
//         {
//             auto location = glGetAttribLocation(_object, record.name.c_str());
//             if( location == - 1 )
//             {
//                 LOGW("failed to locate attribute %s.", record.name.c_str());
//                 continue;
//             }
//             record.location = location;
//         }

//         // unsigned location = get_uniform_location
//         VertexAttribute attribute = record.vb->get_attribute_at(record.attribute_index);
//         if( vbo != 0 )
//         {
//             glEnableVertexAttribArray(record.location);
//             glVertexAttribPointer(record.location,
//                 attribute.size,
//                 GL_ELEMENT_FORMAT[value(attribute.format)],
//                 attribute.normalized,
//                 record.vb->get_stride(),
//                 (uint8_t*)0+attribute.offset);
//         }
//         else
//             glDisableVertexAttribArray(record.location);
//     }
// }

// void Shader::set_vertex_attribute(const char* name, VertexBuffer::ptr vb, unsigned index)
// {
//     ASSERT( name, "invalid attribute name." );

//     if( !_object || _device.is_device_lost() )
//     {
//         LOGW("trying to set vertex attribute while device is lost.");
//         return;
//     }

//     if( !vb )
//     {
//         LOGW("trying to set vertex attribute without a bound VBO.");
//         return;
//     }

//     vertex_record record;
//     record.name = name;
//     record.vb = vb;
//     record.attribute_index = index;

//     auto hash = math::StringHash(name);
//     auto found = _attributes.find(hash);
//     if( found != _attributes.end() )
//     {
//         *found = record;
//         return;
//     }

//     auto location = glGetAttribLocation(_object, name);
//     if( location == - 1 )
//     {
//         LOGW("failed to locate attribute %s.", name);
//         return;
//     }

//     record.location = location;
//     _attributes.insert(std::make_pair(hash, record));
//     CHECK_GL_ERROR();
// }

// int32_t Shader::get_uniform_location(const char* name)
// {
//     if( !_object || _device.is_device_lost() )
//     {
//         LOGW("trying to set uniform while device is lost.");
//         return -1;
//     }

//     auto hash = math::StringHash(name);
//     auto found = _locations.find(hash);
//     if( found != _locations.end() )
//         return *found;

//     auto location = glGetUniformLocation(_object, name);
//     if( location == -1 )
//         LOGW("failed to locate uniform %s.", name);

//     _locations.insert(std::make_pair(hash, location));
//     return location;
// }

// void Shader::set_texture(const char* name, Texture::ptr texture)
// {
//     texture_record record;
//     record.texture = texture;
//     record.name = name;
//     record.hash = math::StringHash(name);

//     auto found = std::find_if(_textures.begin(), _textures.end(), [&](const texture_record& item)
//     {
//         return item.hash == record.hash;
//     });

//     if( found != _textures.end() )
//     {
//         *found = record;
//         return;
//     }

//     _device.set_shader(_object);
//     auto location = get_uniform_location(name);
//     if( location == -1 )
//         return;

//     glUniform1i(location, _textures.get_size());
//     record.location = location;
//     _textures.push_back(record);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform1f(const char* name, const math::Vector<1, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniform1f(pos, value[0]);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform2f(const char* name, const math::Vector<2, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniform2f(pos, value[0], value[1]);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform3f(const char* name, const math::Vector<3, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniform3f(pos, value[0], value[1], value[2]);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform4f(const char* name, const math::Vector<4, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniform4f(pos, value[0], value[1], value[2], value[3]);
//     CHECK_GL_ERROR();
// }

// // OpenGL use column-major layout, so we always transpose our matrix
// void Shader::set_uniform2fm(const char* name, const math::Matrix<2, 2, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniformMatrix2fv(pos, 1, GL_TRUE, (float*)&value);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform3fm(const char* name, const math::Matrix<3, 3, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniformMatrix3fv(pos, 1, GL_TRUE, (float*)&value);
//     CHECK_GL_ERROR();
// }

// void Shader::set_uniform4fm(const char* name, const math::Matrix<4, 4, float>& value)
// {
//     auto pos = get_uniform_location(name);
//     if( pos == -1 )
//         return;

//     _device.set_shader(_object);
//     glUniformMatrix4fv(pos, 1, GL_TRUE, (float*)&value);
//     CHECK_GL_ERROR();
// }

// NS_LEMON_GRAPHICS_END