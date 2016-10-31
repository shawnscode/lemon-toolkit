// @date 2016/10/27
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <codebase/handle_set.hpp>

#include <thread>

NS_LEMON_BEGIN

/**
* @brief      A thread-safe handle based object pool, provides O(1) amortized operations.
*
* @tparam     T     The type of object.
* @tparam     N     The max size of available object.
*/
template<typename T, size_t N> struct HandleObjectSet
{
    using index_t = Handle::index_t;
    using aligned_storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    using array_t = std::array<uint8_t, sizeof(aligned_storage_t)*N>;
    using mutex_t = std::mutex;
    using handle_set_t = HandleSet<N>;

    virtual ~HandleObjectSet();

    /**
     * @brief      Create a constructed object, and a associated unique handle.
     *
     * @param[in]  args  Variadic arguments to construct object.
     *
     * @tparam     Args  The type traits of variadic arguments.
     *
     * @return     Returns associated unique handle.
     */
    template<typename ... Args> Handle create(Args&&... args);

    /**
     * @brief      Fetch object assigned with handle.
     *
     * @param[in]  Handle  The unique handle of object.
     *
     * @return     Returns nullptr_t if no object assigned to this handle.
     */
    T* fetch(Handle handle);

    /**
     * @brief      Determines if the handle and its interanl object is alive
     *
     * @param[in]  Handle  The unique handle of object.
     *
     * @return     True if alive, False otherwise.
     */
    bool is_alive(Handle handle) const;

    /**
     * @brief      Recycle the handle, and its internal object.
     *
     * @param[in]  Handle  The unique handle of object.
     */
    void free(Handle handle);

    /**
     * @brief      Reset this object pool to initial state, and destroy all the objects.
     */
    void clear();

protected:
    T* fetch_without_check(Handle handle);

    array_t _buffer;
    handle_set_t _handles;
};

template<typename T, size_t N>
HandleObjectSet<T, N>::~HandleObjectSet()
{
    clear();
}

template<typename T, size_t N>
template<typename ... Args> Handle HandleObjectSet<T, N>::create(Args&&... args)
{
    if( auto handle = _handles.create() )
    {
        if( auto object = fetch(handle) )
        {
            ::new (object) T(std::forward<Args>(args)...);
            return handle;
        }
    }

    return Handle();
}

template<typename T, size_t N>
INLINE T* HandleObjectSet<T, N>::fetch(Handle handle)
{
    return is_alive(handle) ? fetch_without_check(handle) : nullptr;
}

template<typename T, size_t N>
INLINE T* HandleObjectSet<T, N>::fetch_without_check(Handle handle)
{
    return (T*)_buffer.data()+sizeof(aligned_storage_t)*handle.get_index();
}

template<typename T, size_t N>
INLINE bool HandleObjectSet<T, N>::is_alive(Handle handle) const
{
    return _handles.is_alive(handle);
}

template<typename T, size_t N>
INLINE void HandleObjectSet<T, N>::free(Handle handle)
{
    if( _handles.free(handle) )
    {
        auto object = fetch_without_check(handle);
        object->~T();
    }
}

template<typename T, size_t N>
INLINE void HandleObjectSet<T, N>::clear()
{
    for( auto handle : _handles )
    {
        if( _handles.free(handle) )
        {
            auto object = fetch_without_check(handle);
            object->~T();
        }
    }
}

NS_LEMON_END