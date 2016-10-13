// @date 2016/10/13
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <codebase/type/type_traits.hpp>
#include <codebase/type/tuple.hpp>

#include <type_traits>

NS_LEMON_BEGIN

// a generic, type-safe and discriminated union.
template<typename ... Types> struct VariantResolver;

template<typename ... Types> struct Variant
{
    static_assert(sizeof...(Types) > 0, "template arguments type list of variant can not be empty.");

    constexpr static const size_t size = static_max<sizeof(Types)...>::value;
    constexpr static const size_t align = static_max<alignof(Types)...>::value;
    constexpr static const size_t type_size = sizeof...(Types);

    using resolver_t = VariantResolver<Types...>;
    using tuple_t = typename std::tuple<Types...>;
    using aligned_storage_t = typename std::aligned_storage<size, align>::type;

    Variant() : _index_t(invalid) {}

    // initialize variant with copy constructor
    Variant(const Variant& rhs)
    {
        _index_t = rhs._index_t;
        resolver_t::copy(_index_t, &rhs._data, &_data);
    }

    Variant(Variant&& rhs)
    {
        _index_t = rhs._index_t;
        resolver_t::move(_index_t, &rhs._data, &_data);
    }

    template<typename T, typename Enable = typename std::enable_if<TupleHas<T, tuple_t>::type>::type>
    Variant(T&& v)
    {
        // find the index of this type in the variant type list
        _index_t = type_size - TupleIndex<T, tuple_t>::value - 1;
        ::new (&_data) T(std::forward<T>(v));
    }

    // copy-assignments
    Variant& operator = (const Variant& rhs)
    {
        resolver_t::destroy(_index_t, &_data);
        _index_t = rhs._index_t;
        resolver_t::copy(_index_t, &rhs._data, &_data);
        return *this;
    }

    // move-assignments
    Variant& operator = (Variant&& rhs)
    {
        resolver_t::destroy(_index_t, &_data);
        _index_t = rhs._index_t;
        resolver_t::move(_index_t, &rhs._data, &_data);
        return *this;
    }

    template<typename T, typename Enable = typename std::enable_if<TupleHas<T, tuple_t>::type>::type>
    Variant& operator = (T&& rhs)
    {
        resolver_t::destroy(_index_t, &_data);
        Variant temp(std::forward<T>(rhs));
        _index_t = temp._index_t;
        resolver_t::move(_index_t, &temp._data, &_data);
        return *this;
    }

    //
    template<typename T, typename Enable = typename std::enable_if<TupleHas<T, tuple_t>::type>::type>
    bool is() const
    {
        return _index_t == TupleIndex<T, tuple_t>::value;
    }

    bool is_valid() const
    {
        return _index_t < type_size;
    }

    template<typename T, typename ... Args> void set(Args&& ... args)
    {
        resolver_t::destroy(_index_t, &_data);
        _index_t = type_size - TupleIndex<T, tuple_t>::value - 1;
        new (&_data) T(std::forward<Args>(args)...);
    }

    template<typename T, typename Enable = typename std::enable_if<TupleHas<T, tuple_t>::type>::type>
    T& get()
    {
        const auto index = type_size - TupleIndex<T, tuple_t>::value - 1;
        if( _index_t == index )
            return *reinterpret_cast<T*>(&_data);
        else
            throw std::runtime_error("failed to get T in variant.");
    }

    template<typename T, typename Enable = typename std::enable_if<TupleHas<T, tuple_t>::type>::type>
    const T& get() const
    {
        const auto index = type_size - TupleIndex<T, tuple_t>::value - 1;
        if( _index_t == index )
            return *reinterpret_cast<const T*>(&_data);
        else
            throw std::runtime_error("failed to get T in variant.");
    }

protected:
    constexpr static const size_t invalid = size_t(-1);

    size_t _index_t;
    aligned_storage_t _data;
};

template<typename T, typename ... Types> struct VariantResolver<T, Types...>
{
    static void destroy(size_t index, void* data)
    {
        if( index == sizeof...(Types) )
            reinterpret_cast<T*>(data)->~T();
        else
            VariantResolver<Types...>::destroy(index, data);
    }

    static void move(size_t index, void* from, void* to)
    {
        if( index == sizeof...(Types) )
            new (to) T(std::move(*reinterpret_cast<T*>(from)));
        else
            VariantResolver<Types...>::move(index, from, to);
    }

    static void copy(size_t index, void* from, void* to)
    {
        if( index == sizeof...(Types) )
            new (to) T(*reinterpret_cast<T*>(from));
        else
            VariantResolver<Types...>::move(index, from, to);
    }
};

template<> struct VariantResolver<>
{
    static void destroy(size_t id, void* data) {}
    static void move(size_t id, void* from, void* to) {}
    static void copy(size_t id, void* from, void* to) {}
};

NS_LEMON_END
