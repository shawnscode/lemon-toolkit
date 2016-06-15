// @date 2016/06/15
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <filesystem/path.h>

NS_FLOW2D_BEGIN

struct Path
{
    using Iterator = std::vector<std::string>::iterator;

    Path() {}
    Path(const char* str) { set(str); }
    Path(const std::string& str) { set(str); }
    Path(const Path&) = default;
    Path& operator = (const Path&) = default;

    Path& set(const std::string&);
    Path& concat(const Path&);

    bool        is_empty() const;
    std::string get_str() const;
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

struct DataStream
{
    using Ptr = std::unique_ptr<DataStream>;

    virtual size_t  size() = 0;
    virtual size_t  position() = 0;
    virtual bool    end() = 0;
    virtual void    seek(size_t) = 0;
    virtual size_t  read(void*, size_t) = 0;
    virtual size_t  write(const void*, size_t) = 0;
    virtual void    flush() = 0;
};

struct Archive
{
    virtual ~Archive() {}
    virtual bool            is_exist(const Path&) const = 0;
    virtual DataStream::Ptr open(const Path&) = 0;
};

struct FilesystemArchive : public Archive
{
    FilesystemArchive();
    FilesystemArchive(const char*);
    FilesystemArchive(const std::string&);

    bool is_exist(const Path&) const override;
    DataStream::Ptr open(const Path&) override;

protected:
    filesystem::path m_root;
};

struct ArchiveManager
{
    ~ArchiveManager();

    template<typename T, typename ... Args> void add_archive(Args && ... args);
    DataStream::Ptr open(const Path&);

protected:
    std::vector<Archive*> m_archives;
};

#include <flow2d/core/archive.inl>
NS_FLOW2D_END