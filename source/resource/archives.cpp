// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/archives.hpp>
#include <zlib.h>

NS_FLOW2D_RES_BEGIN

FilesystemArchive::FilesystemArchive(ArchiveCollection& collection,
    const fs::Path& path)
: Archive(collection), _prefix(path)
{}

bool FilesystemArchive::initialize()
{
    if( !_prefix.is_absolute() )
        _prefix = fs::get_current_directory() / _prefix;
    return true;
}

bool FilesystemArchive::is_exist(const fs::Path& path)
{
    return fs::is_regular_file(_prefix / path);
}

std::fstream FilesystemArchive::open(const fs::Path& path, fs::FileMode mode)
{
    return fs::open(_prefix / path, mode);
}

PackageArchive::PackageArchive(ArchiveCollection& collection,
    const fs::Path& path, unsigned offset)
: Archive(collection), _filepath(path), _offset(offset)
{}

PackageArchive::~PackageArchive()
{
    if( _stream.is_open() )
        _stream.close();
}

bool PackageArchive::initialize()
{
    if( _stream.is_open() )
        _stream.close();

    _stream = _collection.open(_filepath, fs::FileMode::READ);
    if( !_stream.is_open() )
    {
        LOGW("failed to open package \"%s\"", _filepath.c_str());
        return false;
    }

    // _stream.seekg(_offset);
    return false;
}

bool PackageArchive::is_exist(const fs::Path&)
{
    return false;
}

std::fstream PackageArchive::open(const fs::Path&, fs::FileMode)
{
    return std::fstream();
}

ArchiveCollection::~ArchiveCollection()
{
    for( auto archive : _archives )
        delete archive;
    _archives.clear();
}

bool ArchiveCollection::add_search_path(const fs::Path& path)
{
    if( fs::is_directory(path) )
        return add_archive<FilesystemArchive>(path);

    LOGW("failed to add \"%s\" as search path, its not a valid directory.",
        path.c_str());
    return false;
}

std::fstream ArchiveCollection::open(const fs::Path& path, fs::FileMode mode)
{
    for( auto archive : _archives )
        if( archive->is_exist(path) )
            return archive->open(path, mode);
    return std::fstream();
}

NS_FLOW2D_RES_END