// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/vertex_array_cache.hpp>
#include <graphics/private/backend.hpp>
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

void VertexArrayObjectCache::bind(ProgramGL& program, VertexBufferGL& vb)
{
    GLuint glprogram = program.get_handle();
    GLuint glvb = vb.get_handle();

    if( glprogram == 0 || glvb == 0 )
    {
        LOGW("failed to bind vertex array object without decent program and vertex_buffer.");
        return;
    }

    if( _vao_support )
    {
        const uint64_t k = (uint64_t)glprogram | (uint64_t)glvb;

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
    auto& layout = vb.get_layout();
    for( uint8_t i = 0; i < VertexAttribute::kVertexAttributeCount; i++ )
    {
        VertexAttribute::Enum va = (VertexAttribute::Enum)i;
        if( layout.has(va) )
        {
            auto localtion = program.get_attribute_location(va);
            if( localtion == -1 )
                continue;

            auto attribute = layout.get_attribute(va);
            glEnableVertexAttribArray(localtion);
            glVertexAttribPointer(
                /*index*/ localtion,
                /*size*/ attribute.size,
                /*type*/ GL_ELEMENT_FORMAT[value(attribute.format)],
                /*normalized*/ attribute.normalized,
                /*stride*/ layout.get_stride(),
                /*pointer*/ (uint8_t*)0+layout.get_offset(va));
        }
    }

    CHECK_GL_ERROR();
}

void VertexArrayObjectCache::unbind()
{
    if( !_vao_support )
        return;

    glBindVertexArray(0);
}

void VertexArrayObjectCache::free(ProgramGL& program)
{
    if( !_vao_support )
        return;

    const uint64_t k = ((uint64_t)program.get_handle()) << 32;
    for( auto it = _vaos.cbegin(); it != _vaos.cend(); )
    {
        if( (it->first & 0xFFFFFFFF00000000) == k )
        {
            glDeleteVertexArrays(1, &it->second);
            _vaos.erase(it++);
        }
        else
            ++it;
    }
    CHECK_GL_ERROR();
}

void VertexArrayObjectCache::free(VertexBufferGL& vb)
{
    if( !_vao_support )
        return;

    const uint64_t k = (uint64_t)vb.get_handle();
    for( auto it = _vaos.cbegin(); it != _vaos.cend(); )
    {
        if( (it->first & 0x00000000FFFFFFFF) == k )
        {
            glDeleteVertexArrays(1, &it->second);
            _vaos.erase(it++);
        }
        else
            ++it;
    }
    CHECK_GL_ERROR();
}

NS_LEMON_GRAPHICS_END
