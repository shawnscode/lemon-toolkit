// @date 2016/06/15
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>

NS_FLOW2D_BEGIN

struct Path
{
    using Iterator = std::vector<std::string>::iterator;

    Path() {}
    Path(const char* str) { set(str); }
    Path(const Path&) = default;
    Path& operator = (const Path&) = default;

    Path& set(const std::string&);
    Path& concat(const Path&);

    bool        is_empty() const;
    std::string get_filename() const;
    std::string get_basename() const;
    std::string get_extension() const;
    Path        get_parent() const;

    Path&   operator /= (const Path&);
    Path&   operator = (Path &&);
    Path    operator / (const Path&) const;
    bool    operator == (const Path&) const;

    Iterator begin();
    Iterator end();

protected:
    friend class PathIterator;
    static std::vector<std::string> tokenize(const std::string&, const std::string& delim);

    std::vector<std::string> m_path;
};

// struct File
// {

// };

// struct Archive
// {

// };

// struct ArchiveManager
// {
//     template<typename T, typename ... Args> void add_archive();
//     void remove_archive();

// protected:
//     std::vector<Archive*> m_archives;
// };

#include <flow2d/core/archive.inl>
NS_FLOW2D_END