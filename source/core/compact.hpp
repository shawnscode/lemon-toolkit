// @date 2016/08/16
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <math/defines.hpp>

NS_LEMON_CORE_BEGIN

template<typename K, typename V, size_t Size> struct CompactHashMap
{
    template<typename T, typename TV> struct iterator_t : public std::iterator<std::bidirectional_iterator_tag, V>
    {
        const static int32_t invalid;

        iterator_t(T&, int32_t pos = invalid);

        iterator_t& operator++();
        iterator_t  operator++(int);
        iterator_t& operator--();
        iterator_t  operator--(int);

        bool operator == (const iterator_t&) const;
        bool operator != (const iterator_t&) const;

        TV& operator * () const;
        int32_t get_cursor() const;

    protected:
        T& _table;
        int32_t _cursor;
    };

    using iterator = iterator_t<CompactHashMap, V>;
    using const_iterator = iterator_t<const CompactHashMap, const V>;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    iterator find(const K&);
    const_iterator find(const K&) const;

    std::pair<iterator, bool> insert(std::pair<K, V>);
    void clear();

protected:
    K _keys[Size];
    V _values[Size];
};

template<typename V, size_t Size> struct CompactVector
{
    template<typename T, typename TV> struct iterator_t : public std::iterator<std::bidirectional_iterator_tag, V>
    {
        const static int32_t invalid;

        iterator_t(T&, int32_t pos = invalid);

        iterator_t& operator++();
        iterator_t  operator++(int);
        iterator_t& operator--();
        iterator_t  operator--(int);

        bool operator == (const iterator_t&) const;
        bool operator != (const iterator_t&) const;

        TV& operator * () const;

    protected:
        T& _vector;
        int32_t _cursor;
    };

    using iterator = iterator_t<CompactVector, V>;
    using const_iterator = iterator_t<const CompactVector, const V>;

    CompactVector() : _size(0) {}
    CompactVector(std::initializer_list<V>);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    V& operator [](unsigned);
    const V& operator [](unsigned) const;

    void        push_back(V&&);
    void        push_back(const V&);

    void        clear();
    unsigned    get_size() const;

protected:
    unsigned    _size;
    V           _values[Size];
};

#include <core/compact.inl>
NS_LEMON_CORE_END