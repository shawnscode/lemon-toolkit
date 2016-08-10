// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/archives.hpp>

NS_FLOW2D_RES_BEGIN

FilesystemArchive::FilesystemArchive(const fs::Path& path)
{
    _prefix = path.is_absolute() ? path : fs::get_current_directory() / path;
}

bool FilesystemArchive::is_exist(const fs::Path& path)
{
    return fs::is_regular_file(_prefix / path);
}

std::fstream FilesystemArchive::open(const fs::Path& path, fs::FileMode mode)
{
    return fs::open(_prefix / path, mode);
}

bool ArchiveCollection::add_search_path(const fs::Path& path)
{
    if( fs::is_directory(path) )
        return add_archive<FilesystemArchive>(path);

    LOGW("failed to add search path of \"%s\"", path.to_string().c_str());
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