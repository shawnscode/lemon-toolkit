// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <codebase/memory/allocator.hpp>
#include <graphics/private/program.hpp>
#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/index_buffer.hpp>
#include <graphics/private/texture.hpp>
#include <graphics/private/drawcall.hpp>
#include <graphics/private/backend.hpp>
#include <graphics/private/vertex_array_cache.hpp>

#include <array>
#include <type_traits>

NS_LEMON_GRAPHICS_BEGIN

// template<typename T, size_t M> struct ResourceAllocator : public FixedBlockAllocator
// {
//     ResourceAllocator() : FixedBlockAllocator(sizeof(T), M) {}

//     template<typename ... Args> Handle create(Args && ... args);
//     T* get(Handle handle);
//     void free(Handle handle);

// protected:
//     std::mutex _mutex;
//     std::array<T, M> _buffer;
//     HandleSet<M> _handle_set;
// };

// struct RendererContext
// {
//     std::mutex mutex;
//     std::vector<RenderDrawcall> drawcalls;
//     VertexArrayObjectCache vaocache;

//     RendererBackend backend;
//     ResourceAllocator<VertexBuffer, kMaxVertexBuffers> vertex_buffers;
//     ResourceAllocator<IndexBuffer, kMaxVertexBuffers> index_buffers;
//     ResourceAllocator<Program, kMaxPrograms> programs;

//     // ResourceAllocator<Texture2D, kMaxTexture2Ds> texture2ds;
// };

// // IMPLEMENTATIONS of RESOURCE ALLOCATOR
// template<typename T, size_t M>
// template<typename ... Args>
// Handle ResourceAllocator<T, M>::create(Args && ... args)
// {
//     Handle handle;

//     {
//         std::unique_lock<std::mutex> L(_mutex);
//         handle = _handle_set.allocate();
//     }

//     if( _handle_set.is_valid(handle) )
//     {
//         if( !_buffer[handle.get_index()].initialize(std::forward<Args>(args)...) )
//             handle.invalidate();
//     }

//     return handle;
// }

// template<typename T, size_t M> T* ResourceAllocator<T, M>::get(Handle handle)
// {
//     if( !_handle_set.is_valid(handle) )
//         return nullptr;

//     return &_buffer[handle.get_index()];
// }

// template<typename T, size_t M> void ResourceAllocator<T, M>::free(Handle handle)
// {
//     if( !_handle_set.is_valid(handle) )
//         return;

//     _buffer[handle.get_index()].dispose();

//     {
//         std::unique_lock<std::mutex> L(_mutex);
//         _handle_set.free(handle);
//     }
// }


NS_LEMON_GRAPHICS_END