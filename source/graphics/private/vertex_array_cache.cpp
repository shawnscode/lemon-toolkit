// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/vertex_array_cache.hpp>
#include <graphics/private/program.hpp>
#include <graphics/private/vertex_buffer.hpp>

NS_LEMON_GRAPHICS_BEGIN

static const unsigned GL_ELEMENT_FORMAT[] =
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_FIXED,
    GL_FLOAT
};

VertexArrayObjectCache::VertexArrayObjectCache()
{
#ifndef GL_ES_VERSION_2_0
    _vao_support = true;
#else
    _vao_support = false;
#endif
}

void VertexArrayObjectCache::bind(Handle hp, Program& program, Handle hv, VertexBuffer& vertex_buffer)
{
    GLuint glprogram = program.get_handle();
    GLuint glvb = vertex_buffer.get_handle();

    if( glprogram == 0 || glvb == 0 )
    {
        LOGW("failed to bind vertex array object without decent program(%d) and vertex_buffer(%d).",
            hp.get_index(), hv.get_index());
        return;
    }

    if( _vao_support )
    {
        const uint64_t pindex   = ((uint64_t)hp.get_index()) << 48;
        const uint64_t pversion = ((uint64_t)hp.get_version()) << 32;
        const uint64_t vindex   = ((uint64_t)hv.get_index()) << 16;
        const uint64_t vversion = ((uint64_t)hv.get_version()) << 0;
        const uint64_t k = pindex | pversion | vindex | vversion;

        auto found = _vaos.find(k);
        if( found != _vaos.end() )
        {
            glBindVertexArray(found->second);
            return;
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        _vaos.insert(std::make_pair(k, vao));
}

    glBindBuffer(GL_ARRAY_BUFFER, glvb);
    auto& attributes = vertex_buffer.get_attributes();
    for( uint8_t i = 0; i < kVertexAttributeCount; i++ )
    {
        auto va = (VertexAttribute)i;
        if( attributes.has(va) )
        {
            auto localtion = glGetAttribLocation(glprogram, get_attribute_name(va));
            if( localtion == -1 )
            {
                LOGW("failed to localte attribute %s.", get_attribute_name(va));
                continue;
            }

            auto attr = attributes.get_attribute(va);

            glEnableVertexAttribArray(localtion);
            glVertexAttribPointer(
                /*index*/ localtion,
                /*size*/ attr.num,
                /*type*/ GL_ELEMENT_FORMAT[value(attr.component)],
                /*normalized*/ attr.normalize,
                /*stride*/ attributes.get_stride(),
                /*pointer*/ (uint8_t*)0+attributes.get_offset(va));
        }
    }

    CHECK_GL_ERROR();
}

void VertexArrayObjectCache::unbind()
{
    if( _vao_support )
        glBindVertexArray(0);
}

void VertexArrayObjectCache::free_vertex_buffer(Handle handle)
{
    const uint64_t k = (((uint64_t)handle.get_index()) << 16) | (((uint64_t)handle.get_version()) << 0);
    for( auto it = _vaos.cbegin(); it != _vaos.cend(); )
    {
        if( (it->first & 0xFFFFFFFF00000000) == k )
            _vaos.erase(it++)       ;
        else
            ++it;
    }
}

void VertexArrayObjectCache::free_program(Handle handle)
{
    const uint64_t k = (((uint64_t)handle.get_index()) << 48) | (((uint64_t)handle.get_version()) << 32);
    for( auto it = _vaos.cbegin(); it != _vaos.cend(); )
    {
        if( (it->first & 0xFFFFFFFF00000000) == k )
            _vaos.erase(it++)       ;
        else
            ++it;
    }
}

NS_LEMON_GRAPHICS_END
