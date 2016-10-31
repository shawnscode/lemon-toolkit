// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <functional>

NS_LEMON_BEGIN

//
// Its a common practice to use a unique id for resource management. This has 
// serveral advantages:
// 1. it would allow for the resource to be destroyed without leaving dangling
// pointers;
// 2. it helps for saving state externally;
// 3. data can be shuffled around in memory as needed.
//

/**
 * @brief      A versioned uniqued identifier.
 */
struct Handle
{
    using index_t = uint16_t;
    const static index_t invalid = uint16_t(-1);

    Handle() = default;
    Handle(const Handle&) = default;
    Handle(index_t index, index_t version);

    Handle& operator = (const Handle&) = default;

    index_t get_index() const;
    index_t get_version() const;
    
    /**
     * @brief      Determines if this handle is valid, both _index and _version not equals invalid.
     *
     * @return     True if valid, False otherwise.
     */
    bool is_valid() const;

    /**
     * @brief      Boolean conversion operator for fast check the validation of handle.
     *
     * @return     True if valid, False otherwise.
     */
    operator bool () const;
    
    /**
     * @brief      Comparisons of equality with other handle
     *
     * @param[in]  rhs   The right hand side handle
     *
     * @return     Returns true if them have same _index and _version value
     */
    bool operator == (const Handle& rhs) const;
    bool operator != (const Handle& rhs) const;

    /**
     * @brief      Comparisons with other handle, its used for sorting containers.
     *
     * @param[in]  rhs   The right hand side handle
     *
     * @return     Returns true we are less than other in strict partially order.
     */
    bool operator < (const Handle& rhs) const;

    /**
     * @brief      Invalidate this handle, reset _index and _version to invalid
     */
    void invalidate();

protected:
    index_t _index = invalid;
    index_t _version = invalid;
};

///

INLINE Handle::Handle(index_t index, index_t version)
: _index(index), _version(version)
{}

INLINE Handle::index_t Handle::get_index() const
{
    return _index;
}

INLINE Handle::index_t Handle::get_version() const
{
    return _version;
}

INLINE bool Handle::is_valid() const
{
    return _index != invalid && _version != invalid;
}

INLINE Handle::operator bool () const
{
    return is_valid();
}

INLINE bool Handle::operator == (const Handle& rhs) const
{
    return _index == rhs._index && _version == rhs._version;
}

INLINE bool Handle::operator != (const Handle& rhs) const
{
    return !(*this == rhs);
}

INLINE bool Handle::operator < (const Handle& rhs) const
{
    return _index == rhs._index ? _version < rhs._version : _index < rhs._index;
}

INLINE void Handle::invalidate()
{
    _index = invalid;
    _version = invalid;
}

NS_LEMON_END

namespace std
{
    template<> struct hash<lemon::Handle>
    {
        std::size_t operator() (const lemon::Handle& handle) const
        {
            return (((size_t)handle.get_version()) << 16) & ((size_t)handle.get_index());
        }
    };

    template<> struct hash<std::pair<lemon::Handle, lemon::Handle>>
    {
        std::size_t operator() (const std::pair<lemon::Handle, lemon::Handle>& value) const
        {
            return std::hash<lemon::Handle>()(value.first) ^ std::hash<lemon::Handle>()(value.second);
        }
    };
}