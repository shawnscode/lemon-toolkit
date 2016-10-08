// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <codebase/memory/allocator.hpp>
#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/index_buffer.hpp>
#include <graphics/private/uniform_buffer.hpp>

#include <array>
#include <type_traits>

NS_LEMON_GRAPHICS_BEGIN

enum class FrontendCommand : uint8_t
{
    CreateVertexBuffer,
    CreateIndexBuffer,
    End
};

// plain command data stream
struct CommandStream
{
    void write(const uint8_t* data, size_t size)
    {
        ASSERT(_position + size < kMaxVertexBuffers,
            "failed to write to command stream (pos: %d, size: %d)", _position, size+1);

        memcpy(&_buffer[_position], data, size);
        _position += size;
    }

    template<typename T> void write(const T& in)
    {
        static_assert(std::is_pod<T>::value, "plain old data requires.");
        write(reinterpret_cast<const uint8_t*>(&in), sizeof(T));
    }

    template<typename T> void read(T& out)
    {
        const static size_t size = sizeof(T);
        static_assert(std::is_pod<T>::value, "plain old data requires.");
        ASSERT(_position + size < _size,
            "failed to read from command stream (pos: %d, size: %d)", _position, _size);

        memcpy(reinterpret_cast<uint8_t*>(&out), &_buffer[_position], size);
    }

    void start_read()
    {
        _size = _position;
        _position = 0;
    }

protected:
    size_t _position = 0;
    size_t _size = 0;
    uint8_t _buffer[kMaxCommandBufferSize];
};

template<typename T, size_t M> struct ResourceAllocator : public FixedBlockAllocator
{
    ResourceAllocator() : FixedBlockAllocator(sizeof(T), M) {}

    template<typename ... Args> Handle create(Args && ... args)
    {
        Handle handle;

        {
            std::unique_lock<std::mutex> L(_mutex);
            handle = _handle_set.allocate();
        }

        if( _handle_set.is_valid(handle) )
        {
            if( !_buffer[handle.get_index()].initialize(std::forward<Args>(args)...) )
                handle.invalidate();
        }

        return handle;
    }

    T* get(Handle handle)
    {
        if( !_handle_set.is_valid(handle) )
            return nullptr;

        return &_buffer[handle.get_index()];
    }

    void free(Handle handle)
    {
        if( !_handle_set.is_valid(handle) )
            return;

        _buffer[handle.get_index()].dispose();

        {
            std::unique_lock<std::mutex> L(_mutex);
            _handle_set.free(handle);
        }
    }

protected:
    std::mutex _mutex;
    std::array<T, M> _buffer;
    HandleSet<M> _handle_set;
};

struct FrontendContext
{
    ResourceAllocator<VertexBuffer, kMaxVertexBuffers> vertex_buffers;
    ResourceAllocator<IndexBuffer, kMaxVertexBuffers> index_buffers;
    ResourceAllocator<UniformBuffer, kMaxVertexBuffers> uniform_buffers;
};

NS_LEMON_GRAPHICS_END