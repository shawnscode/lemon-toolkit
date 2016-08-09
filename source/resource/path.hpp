// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/defines.hpp>

NS_FLOW2D_RES_BEGIN

// operation system independent path.
struct Path
{
    constexpr static const char * const sperator = "/";
    constexpr static const char * const extension_sperator = ".";
    constexpr static const char * const previous = "..";

    Path() {}
    Path(const char* str) { set(std::move(std::string(str))); }
    Path(const std::string& str) { set(str); }
    Path(std::string&& str) { set(std::move(str)); }

    Path(const Path&) = default;
    Path(Path&& rhs) { _pathname = std::move(rhs._pathname); }
    Path& operator = (const Path&) = default;
    Path& operator = (Path&& rhs) { _pathname = std::move(rhs._pathname); return *this; }

    Path& set(const std::string& rhs) { _pathname = rhs; tokenize(); return *this; }
    Path& set(std::string&& rhs) { _pathname = std::move(rhs); tokenize(); return *this; }
    Path& concat(const Path&);

    bool is_empty() const { return _pathname.empty(); }
    bool is_absolute() const { return _pathname.compare(0, strlen(Path::sperator), Path::sperator) == 0; }

    // return filename of this path
    std::string get_filename() const;
    // return get_filename() without extension
    std::string get_basename() const;
    // return extension if get_filename() contains a dot
    std::string get_extension() const;
    // return string representation of path
    const std::string& get_string() const { return _pathname; }
    // return parent/root path of this
    Path        get_root() const;
    Path        get_parent() const;

    // operator overloading of concatenation
    Path&   operator /= (const Path& rhs) { return concat(rhs); }
    Path    operator / (const Path& rhs) const { Path result = *this; return result /= rhs; }
    // comparisons of path
    bool    operator == (const Path& rhs) const { return _pathname == rhs._pathname; }
    bool    operator == (const char* rhs) const { return *this == Path(rhs); }

    struct iterator : public std::iterator<std::bidirectional_iterator_tag, Path>
    {
        iterator(const Path& p, size_t pos = std::string::npos);

        iterator& operator ++ ();
        iterator operator ++ (int);
        iterator& operator -- ();
        iterator operator -- (int);

        bool operator == (const iterator&) const;
        bool operator != (const iterator&) const;
        std::string operator * () const;

    protected:
        const Path& _target;
        size_t      _iterator;
    };
    using const_iterator = iterator;

    iterator begin() { return Path::iterator(*this, 0); }
    iterator end() { return Path::iterator(*this); }
    const_iterator begin() const { return Path::iterator(*this, 0); }
    const_iterator end() const { return Path::iterator(*this); }

protected:
    void tokenize();
    void compress();

    std::string _pathname;
};

std::ostream& operator << (std::ostream&, const Path&);
std::string to_string(const Path&);

NS_FLOW2D_RES_END
