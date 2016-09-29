// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/backend.hpp>
// #include <graphics/vertex_buffer.hpp>
#include <graphics/texture.hpp>
#include <math/matrix.hpp>
#include <math/string_hash.hpp>

NS_LEMON_GRAPHICS_BEGIN

// struct Shader : public GraphicsObject
// {
//     using ptr = std::shared_ptr<Shader>;
//     using weak_ptr = std::weak_ptr<Shader>;

//     Shader(Backend& device) : GraphicsObject(device) {}
//     virtual ~Shader() { release(); }

//     bool restore(const char* vs, const char* ps);
//     bool restore() override;
//     void release() override;

//     // bind this shader and corresponding states to graphic device
//     void use();

//     // set shader's uniform by name
//     void set_uniform1f(const char*, const math::Vector<1, float>&);
//     void set_uniform2f(const char*, const math::Vector<2, float>&);
//     void set_uniform3f(const char*, const math::Vector<3, float>&);
//     void set_uniform4f(const char*, const math::Vector<4, float>&);
//     void set_uniform2fm(const char*, const math::Matrix<2, 2, float>&);
//     void set_uniform3fm(const char*, const math::Matrix<3, 3, float>&);
//     void set_uniform4fm(const char*, const math::Matrix<4, 4, float>&);

//     // set texture uniform with unit index
//     void set_texture(const char*, Texture::ptr);
//     // set vertex attribute
//     void set_vertex_attribute(const char*, VertexBuffer::ptr, unsigned);

// protected:
//     struct vertex_record
//     {
//         std::string         name;
//         VertexBuffer::ptr   vb;
//         unsigned            attribute_index;
//         int                 location = 0;
//     };

//     struct texture_record
//     {
//         std::string         name;
//         math::StringHash    hash;
//         Texture::ptr        texture;
//         int                 location = 0;
//     };

//     using attribute_table = core::CompactHashMap<math::StringHash, vertex_record, kMaxVertexAttributes>;
//     using texture_vector  = core::CompactVector<texture_record, kMaxTextures>;
//     using localtion_table = core::CompactHashMap<math::StringHash, int32_t, kMaxUniforms>;

//     int32_t get_uniform_location(const char*);

//     std::string     _fragment_shader;
//     std::string     _vertex_shader;
//     attribute_table _attributes;
//     texture_vector  _textures;
//     localtion_table _locations;
// #ifndef GL_ES_VERSION_2_0
//     unsigned _vao = 0;
// #endif
// };

NS_LEMON_GRAPHICS_END