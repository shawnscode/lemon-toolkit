#include <core/archive.hpp>
#include <cstdio>
#include <cerrno>
#include <sstream>

NS_FLOW2D_BEGIN

std::string Path::get_str() const
{
    std::ostringstream oss;
    oss << "./";
    for( auto i=0; i<m_path.size(); i++ )
    {
        if( i < (m_path.size()-1) ) oss << m_path[i] << "/";
        else oss << m_path[i];
    }
    return oss.str();
}

ArchiveManager::~ArchiveManager()
{
    for( auto archive : m_archives )
        delete archive;
}

filesystem::path operator / (const filesystem::path& lh, const Path& rh)
{
    return lh / filesystem::path { rh.get_str() };
}

struct FilesystemDataStream : public DataStream
{
    FilesystemDataStream(FILE* file)
    : handle(file)
    {}

    virtual ~FilesystemDataStream()
    {
        fclose(handle);
        handle = nullptr;
    }

    size_t size() override
    {
        auto pos = ftell(handle);
        ASSERT( pos != -1, "ftell failed: errno = %d", errno );
        auto err = fseek(handle, 0, SEEK_END);
        ASSERT( err == 0, "fseek failed: errno = %d", errno );
        auto size = ftell(handle);
        ASSERT( size != -1, "ftell failed: errno = %d", errno );
        err = fseek(handle, (long)pos, SEEK_SET);
        ASSERT( err == 0, "fseek failed: errno = %d", errno );
        return (size_t)size;
    }

    size_t position() override
    {
        auto pos = ftell(handle);
        ASSERT( pos != -1, "ftell failed: errno = %d", errno );
        return (size_t)pos;
    }

    bool end() override
    {
        return feof(handle) != 0;
    }

    void seek(size_t position) override
    {
        auto err = fseek(handle, (long)position, SEEK_SET);
        ASSERT( err == 0, "fseek failed: errno = %d", errno );
    }

    size_t read(void* data, size_t size) override
    {
        auto bytes_read = fread(data, 1, size, handle);
        ASSERT( ferror(handle) == 0, "fread failed: errno = %d", errno );
        return (size_t)bytes_read;
    }

    size_t write(const void* data, size_t size) override
    {
        auto bytes_written = fwrite(data, 1, size, handle);
        ASSERT( ferror(handle) == 0, "fwrite failed: errno = %d", errno );
        return (size_t)bytes_written;
    }

    void flush() override
    {
        auto err = fflush(handle);
        ASSERT( err == 0, "fwrite failed: errno = %d", errno );
    }

protected:
    FILE* handle;
};

FilesystemArchive::FilesystemArchive()
: m_root(filesystem::path::getcwd())
{}

FilesystemArchive::FilesystemArchive(const char* str)
: m_root(str)
{}

FilesystemArchive::FilesystemArchive(const std::string& str)
: m_root(str)
{}

bool FilesystemArchive::is_exist(const Path& path) const
{
    return (m_root / path).exists();
}

DataStream::Ptr FilesystemArchive::open(const Path& path)
{
    filesystem::path fullpath = m_root / path;
    if( fullpath.exists() )
    {
        auto fp = fopen(fullpath.str().c_str(), "r");
        if( fp == nullptr )
        {
            LOGI("failed to open file at %s.", fullpath.str().c_str());
            return { nullptr };
        }

        DataStream* stream = new (std::nothrow) FilesystemDataStream(fp);
        return DataStream::Ptr(stream);
    }

    return { nullptr };
}

NS_FLOW2D_END