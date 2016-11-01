// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <cstdlib>
#include <atomic>

NS_LEMON_GRAPHICS_BEGIN

enum class FrameOperation : uint8_t
{
    PRE,
    POST
};

struct FrameTask
{
    virtual void dispatch(RenderBackend&) = 0;
};

struct RenderFrame
{
    RenderFrame(size_t packet_size, size_t buffer_size)
    {
        _packet_size = packet_size;
        _buffer_size = buffer_size;

        _packet_tail.store(0);
        _packets.reset( new (std::nothrow) FrameTask*[packet_size] );

        _buffer_tail.store(0);
        _buffer.reset( new (std::nothrow) uint8_t[buffer_size] );
    }

    template<typename T> T* make()
    {
        using aligned_storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

        auto index = _packet_tail.fetch_add(1);
        auto object = static_cast<T*>(allocate(sizeof(aligned_storage_t)));
        ::new (object) T();
        _packets[index] = object;
        return object;
    }

    void* allocate(size_t size)
    {
        if( _buffer_tail.load() + size > _buffer_size  )
        {
            return nullptr;
        }
        else
        {
            auto start = _buffer_tail.fetch_add(size);
            return static_cast<void*>(_buffer.get() + start);
        }
    }

    void clear()
    {
        _packet_tail.store(0);
        _buffer_tail.store(0);
    }

    size_t _packet_size;
    size_t _buffer_size;

    std::atomic<size_t> _packet_tail;
    std::unique_ptr<FrameTask*[]> _packets;

    std::atomic<size_t> _buffer_tail;
    std::unique_ptr<uint8_t[]> _buffer;
};


NS_LEMON_GRAPHICS_END
