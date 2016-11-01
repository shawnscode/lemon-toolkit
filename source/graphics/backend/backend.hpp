// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <graphics/graphics.hpp>
#include <graphics/state.hpp>
#include <graphics/backend/opengl.hpp>

#include <math/rect.hpp>
#include <math/color.hpp>
#include <math/string_hash.hpp>

#include <unordered_map>

struct SDL_Window;

NS_LEMON_GRAPHICS_BEGIN

struct VertexBufferGL
{
    void create(const void* data, size_t size, const VertexLayout&, GLenum usage);
    void update(GLuint start, const void* data, size_t size);
    void free();

    GLuint _uid = 0;
    GLuint _num = 0;
    GLenum _usage = GL_STATIC_DRAW;
    VertexLayout _layout;
};

struct IndexBufferGL
{
    void create(const void* data, size_t size, GLuint element_size, GLenum usage);
    void update(GLuint start, const void* data, size_t size);
    void free();

    GLuint _uid = 0;
    GLuint _num = 0;
    GLuint _element_size = 0;
    GLenum _usage = GL_STATIC_DRAW;
};

struct ProgramGL
{
    using pair_t = std::pair<math::StringHash, GLint>;
    using tex_pair_t = std::pair<math::StringHash, Handle>;

    void create(const char* vs, const char* ps);
    void free();

    GLint bind_attribute(VertexAttribute::Enum va, const char* name);
    GLint bind_uniform(const char* name);
    void update_uniform(math::StringHash, const UniformVariable&);

    GLuint _uid = 0;

    uint8_t _uniform_size = 0;
    pair_t _uniforms[kMaxUniformsPerMaterial];

    uint8_t _texture_size = 0;
    tex_pair_t _textures[kMaxTexturePerMaterial];

    pair_t _attributes[VertexAttribute::kVertexAttributeCount];
};

struct TextureGL
{
    void create(const void*, GLenum, GLenum, uint16_t, uint16_t, GLenum);
    void update_mipmap(bool mipmap);
    void update_address_mode(int8_t, GLenum);
    void update_filter_mode(GLenum);
    void update_parameters();
    void free();

    bool _mipmap = false, _dirty = false;
    uint16_t _width, _height;
    GLenum _usage = GL_STATIC_DRAW;
    GLenum _format = GL_ALPHA;
    GLenum _pixel_format = GL_UNSIGNED_BYTE;
    GLenum _filter = GL_LINEAR;
    GLenum _address[3];

    GLuint _uid = 0;
};

// graphics device subsystem. manages the window device, renedering state and gpu resources
struct RenderBackend
{
    virtual ~RenderBackend() {}

    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool initialize(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void dispose();

    // reset all the graphics state to default
    void reset_cached_state();

    // begin frame rendering. return true if device available and can reneder
    bool begin_frame();
    // end frame rendering and swap buffers
    void end_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const math::Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);

    // Creates static vertex buffer.
    void create_vertex_buffer(Handle, const void*, size_t, const VertexLayout&, BufferUsage);
    // Update dynamic(BufferUsage::DYNAMIC) vertex buffer.
    void update_vertex_buffer(Handle, uint32_t, const void*, size_t);
    // Destroy vertex buffer.
    void free_vertex_buffer(Handle);

    // Creates an index buffer. 
    void create_index_buffer(Handle, const void*, size_t, IndexElementFormat, BufferUsage);
    // Update dynamic(BufferUsage::DYNAMIC) index buffer.
    void update_index_buffer(Handle, uint32_t, const void*, size_t);
    // Destroy index buffer.
    void free_index_buffer(Handle);

    // Create texture.
    void create_texture(Handle, const void*, TextureFormat, TexturePixelFormat, uint16_t, uint16_t, BufferUsage);
    // Update texture mipmap.
    void update_texture_mipmap(Handle, bool);
    // Update texture address mode.
    void update_texture_address_mode(Handle, TextureCoordinate, TextureAddressMode);
    // Update texture filter mode.
    void update_texture_filter_mode(Handle, TextureFilterMode);
    // Destroy texture.
    void free_texture(Handle);

    // Compile and link shaders.
    void create_program(Handle, const char*, const char*);
    // Destroy program.
    void free_program(Handle);

    // Create uniform %name associated with program.
    void create_program_uniform(Handle, const char*);
    void update_program_uniform(Handle, math::StringHash, const UniformVariable&);
    // Create attribute %name associated with program.
    void create_program_attribute(Handle, VertexAttribute::Enum, const char*);

    // set the viewport
    void set_viewport(const math::Rect2i&);
    // specify whether front- or back-facing polygons can be culled
    void set_cull_face(bool, CullFace);
    // define front- and back-facing polygons
    void set_front_face(FrontFaceOrder);
    // define the scissor box
    void set_scissor_test(bool, const math::Rect2i& scissor = {{0, 0}, {0, 0}});
    // set front and back function and reference value for stencil testing
    void set_stencil_test(bool, CompareEquation, unsigned reference, unsigned mask);
    // set front and back stencil write actions
    void set_stencil_write(StencilWriteEquation sfail, StencilWriteEquation dpfail, StencilWriteEquation dppass, unsigned mask);
    // specify the value used for depth buffer comparisons
    void set_depth_test(bool, CompareEquation);
    // enable or disable writing into the depth buffer with bias
    void set_depth_write(bool, float slope_scaled = 0.f, float constant = 0.f);
    // set blending mode
    void set_color_blend(bool, BlendEquation, BlendFactor, BlendFactor);
    // enable and disable writing of frame buffer color components
    void set_color_write(ColorMask);

    // set texture
    void set_texture(unsigned, unsigned, unsigned);

    // set current shader program
    void set_program(Handle);
    // set index buffer
    void set_index_buffer(Handle);
    // set vertex buffer
    void set_vertex_buffer(Handle);

    // draw geometry
    void draw(PrimitiveType, uint32_t start, uint32_t count);

    // check if we have valid window and OpenGL context
    bool is_device_lost() const;

protected:
    void set_attribute_layout(Handle material, Handle vb);
    void set_texture_layout(Handle material);

protected:
    using vao_table_t = std::unordered_map<std::pair<Handle, Handle>, GLuint>;

    SDL_Window* _window = nullptr;
    void* _context = 0;
    int32_t _system_frame_object = 0;

    // render states
    RenderState _render_state;
    math::Rect2i _viewport;

    unsigned _bound_fbo;
    unsigned _active_texunit = 0;
    unsigned _bound_texture = 0;
    unsigned _bound_textype = 0;

    Handle _active_material;
    Handle _active_vbo;
    Handle _active_ibo;

    ProgramGL _materials[kMaxProgram];
    IndexBufferGL _ibs[kMaxIndexBuffer];
    VertexBufferGL _vbs[kMaxVertexBuffer];
    TextureGL _textures[kMaxTexture];

    // vao cache
    bool _vao_support = true;
    std::pair<Handle, Handle> _active_vao;
    vao_table_t _vao_cache;
};

NS_LEMON_GRAPHICS_END
