// @date 2016/08/05
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/filesystem.hpp>

#ifdef PLATFORM_WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#endif

NS_FLOW2D_FS_BEGIN

static void replace(std::string& str, const char* from, const char* to)
{
    unsigned size = strlen(from);
    if( size == 0 )
        return;

    unsigned size_to = strlen(to);
    size_t start_pos = 0;
    while( (start_pos = str.find(from, start_pos)) != std::string::npos )
    {
        str.replace(start_pos, size, to);
        start_pos += size_to;
    }
}

#ifdef PLATFORM_WIN32
static std::string to_unix(const wchar_t* wstr)
{
    std::string result;
    auto len = wcslen(wstr);
    if( len > 0 )
    {
        auto size = WideCharToMultiByte(CP_UTF8, 0, wstr, len, nullptr, 0, nullptr, nullptr);
        result.resize(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, len, &result, size, nullptr, nullptr);
    }

    replace(result, "\\", Path::sperator);
    return std::move(result);
}

static std::wstring to_win32(const char* str)
{
    std::wstring result;
    auto len = strlen(str);
    if( len > 0 )
    {
        int size = MultiByteToWideChar(CP_UTF8, 0, &result[0], (int)result.size(), NULL, 0);
        result.resize(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &result[0], (int)result.size(), &result[0], size);
    }

    replace(result, Path::sperator, "\\");
    return std::move(result);
}
#endif

int get_filemode(FileMode mode)
{
    int mask = 0;
    if( to_value(mode & FileMode::READ) )
        mask |= std::ios::in;

    if( to_value(mode & FileMode::WRITE) )
        mask |= std::ios::out;

    if( to_value(mode & FileMode::APPEND) )
        mask |= std::ios::app;

    if( to_value(mode & FileMode::BINARY) )
        mask |= std::ios::binary;

    if( to_value(mode & FileMode::ATEND) )
        mask |= std::ios::ate;

    if( to_value(mode & FileMode::TRUNCATE) )
        mask |= std::ios::trunc;

    return mask;
}

///
static Path s_current_dir;

const Path& get_current_directory()
{
    if( s_current_dir.is_empty() )
    {
#ifdef PLATFORM_WIN32
        wchar_t path[kMaxPath];
        path[0] = 0;
        GetCurrentDirectory(kMaxPath, path);
        s_current_dir.set(to_unix(path));
#else
        char path[kMaxPath];
        path[0] = 0;
        getcwd(path, kMaxPath);
        s_current_dir.set(path);
#endif
    }

    return s_current_dir;
}

bool set_current_directory(const Path& path)
{
    auto nwd = path.is_absolute() ? path : get_current_directory() / path;

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(nwd.get_string().c_str());
    if( !SetCurrentDirectoryW(wstring_path.c_str()) )
#else
    if( chdir(nwd.get_string().c_str()) != 0 )
#endif
    {
        LOGW("failed to set working directory to \"%s\".", nwd.get_string().c_str());
        return false;
    }

    s_current_dir = std::move(nwd);
    return true;
}

bool create_directory(const Path& path, bool recursive)
{
    if( path.is_empty() )
    {
        LOGW("trying to create directory with empty path.");
        return false;
    }

    auto parent = path.get_parent();
    if( !is_directory(parent) )
    {
        if( !recursive )
        {
            LOGW("failed to create directory due to absence of parent directory");
            return false;
        }

        if( !create_directory(parent, recursive) )
            return false;
    }

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(path.get_string().c_str());
    if( CreateDirectoryW(wstring_path.c_str(), nullptr) )
#else
    if( mkdir(path.get_string().c_str(), S_IRWXU) != 0 && errno != EEXIST )
#endif
    {
        LOGW("failed to create directory \"%s\".", path.get_string().c_str());
        return false;
    }

    return true;
}

bool move(const Path& from, const Path& to)
{
    if( from.is_empty() || to.is_empty() )
    {
        LOGW("trying to move file with empty path.");
        return false;
    }

#ifdef PLATFORM_WIN32
    auto from_wstr = to_win32(from.get_string().c_str());
    auto to_wstr = to_win32(to.get_string().c_str());
    return MoveFileW(from_wstr.c_str(), to_wstr.c_str()) != 0;
#else
    return std::rename(from.get_string().c_str(), to.get_string().c_str()) == 0;
#endif
}

bool remove(const Path& path, bool recursive)
{
    if( path.is_empty() )
    {
        LOGW("trying to remove file with empty path.");
        return false;
    }

    if( is_directory(path) && recursive )
    {
        auto success = true;
        auto mode = ScanMode::HIDDEN | ScanMode::DIRECTORIES | ScanMode::FILES | ScanMode::RECURSIVE;
        for( auto iter : scan(path, mode) )
        {
#ifdef PLATFORM_WIN32
            auto wstring_path = to_win32(iter.get_string().c_str());
            success &= DeleteFileW(wstring_path.c_str());
#else
            success &= (std::remove(iter.get_string().c_str()) == 0);
#endif
        }

        if( !success ) return false;
    }

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(path.get_string().c_str());
    return DeleteFileW(wstring_path.c_str());
#else
    return std::remove(path.get_string().c_str()) == 0;
#endif
}

bool is_exists(const Path& path)
{
    if( path.is_empty() )
        return true;

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(path.get_string().c_str());
    DWORD attr = GetFileAttributesW(wstring_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES;
#else
    struct stat sb;
    return stat(path.get_string().c_str(), &sb) == 0;
#endif
}

bool is_regular_file(const Path& path)
{
    if( path.is_empty() )
        return false;

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(path.get_string().c_str());
    DWORD attr = GetFileAttributesW(wstring_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY == 0);
#else
    struct stat sb;
    if( stat(path.get_string().c_str(), &sb) )
        return false;
    return S_ISREG(sb.st_mode);
#endif
}

bool is_directory(const Path& path)
{
    if( path.is_empty() )
        return true;

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(path.get_string().c_str());
    DWORD attr = GetFileAttributesW(wstring_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY != 0);
#else
    struct stat sb;
    if( stat(path.get_string().c_str(), &sb) )
        return false;
    return S_ISDIR(sb.st_mode);
#endif
}

std::fstream open(const Path& path, FileMode mode)
{
    std::fstream fs;
#ifdef PLATFORM_WIN32
    fs.open(to_win32(path.get_string().c_str()).c_str(), get_filemode(mode));
#else
    fs.open(path.get_string().c_str(), get_filemode(mode));
#endif

    if( !fs.is_open() )
        LOGW("failed to open file \"%s\" with mode %d",
            path.get_string().c_str(),(uint16_t)mode);

    return fs;
}

Directory scan(const Path& path, ScanMode mode)
{
    Directory directory;
    if( !directory.scan(path, mode) )
        LOGW("failed to scan directory \"%s\"", path.get_string().c_str());
    return directory;
}

#ifdef PLATFORM_WIN32
static bool scan_directory(std::vector<Path>& nodes, const Path& path, ScanMode mode)
{
    WIN32_FIND_DATAW info;
    HANDLE handle = FindFirstFileW(to_win32(path.get_string().c_str()).c_str(), &info);
    if( handle != INVALID_HANDLE_VALUE )
    {
        do
        {
            auto npath = (path / info.cFileName);
            if( !info.cFileName.empty() )
            {
                if( info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(mode & ScanMode::HIDDEN) )
                    continue;
                if( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    if( to_value(mode & ScanMode::RECURSIVE) )
                        scan_directory(nodes, npath, mode);

                    if( to_value(mode & ScanMode::DIRECTORIES) )
                        nodes.push_back(std::move(npath));
                }
                else if( to_value(mode & ScanMode::FILES) )
                    nodes.push_back(std::move(npath));
            }
        }
        while(FindNextFileW(handle, &info));
        FindClose(handle);
        return true;
    }
    return false;
}
#else
static bool scan_directory(std::vector<Path>& nodes, const Path& path, ScanMode mode)
{
    auto dir = opendir(path.get_string().c_str());
    if( dir )
    {
        struct dirent* de;
        struct stat st;
        while( (de = readdir(dir)) )
        {
            if( strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 )
                continue;

            if( !to_value(mode & ScanMode::HIDDEN) && de->d_name[0] == '.' )
                continue;

            auto npath = (path / de->d_name);
            if( stat(npath.get_string().c_str(), &st) == 0 )
            {
                if( st.st_mode & S_IFDIR )
                {
                    if( to_value(mode & ScanMode::RECURSIVE) )
                        scan_directory(nodes, npath, mode);

                    if( to_value(mode & ScanMode::DIRECTORIES) )
                        nodes.push_back(std::move(npath));
                }
                else if( to_value(mode & ScanMode::FILES) )
                    nodes.push_back(std::move(npath));
            }
        }
        closedir(dir);
        return true;
    }
    return false;
}
#endif

bool Directory::scan(const Path& path, ScanMode mode)
{
    return scan_directory(_nodes, path, mode);
}

NS_FLOW2D_FS_END