// @date 2016/08/05
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/filesystem.hpp>

#ifdef PLATFORM_WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif

NS_FLOW2D_RES_BEGIN

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
static const wchar_t* get_filemode(FileMode mode)
{
    if( mode & FileMode::READ )
    {
        if( mode & FileMode::APPEND ) return L"a+";
        else if( mode & FileMode::WRITE ) return L"r+";
        else return L"r";
    }
    else if( mode & FileMode::APPEND ) return L"a";
    else if( mode & FileMode::WRITE) return L"w";
    return L"";
}
#else
static const char* get_filemode(FileMode mode)
{
    if( mode & FileMode::READ )
    {
        if( mode & FileMode::APPEND ) return "a+";
        else if( mode & FileMode::WRITE ) return "r+";
        else return "r";
    }
    else if( mode & FileMode::APPEND ) return "a";
    else if( mode & FileMode::WRITE) return "w";
    return "";
}
#endif

////
bool File::open(const Path& path, FileMode mode)
{
    auto string_path = to_string(path.is_absolute() ?
        path : _filesystem.get_working_directory() / path);

#ifdef PLATFORM_WIN32
    _object = _wfopen(to_win32(string_path).c_str(), get_filemode(mode));
#else
    _object = fopen(string_path.c_str(), get_filemode(mode));
#endif

    if( !_object )
    {
        LOGW("failed to open file \"%s\".", string_path.c_str());
        return false;
    }

    _mode = mode;
    return true;
}

void File::close()
{
    if( _object )
    {
        fclose((FILE*)_object);
        _object = nullptr;
    }
}

unsigned File::read(void* dst, unsigned size)
{
    auto bytes_read = fread(dst, 1, size, (FILE*)_object);
    ASSERT( ferror((FILE*)_object) == 0, "fread failed: errno = %d", errno );
    return (size_t)bytes_read;
}

void File::seek(unsigned position)
{
    auto err = fseek((FILE*)_object, (long)position, SEEK_SET);
    ASSERT( err == 0, "fseek failed: errno = %d", errno );
}

bool File::is_end() const
{
    return feof((FILE*)_object) != 0;
}

unsigned File::write(const void* src, unsigned size)
{
    auto bytes_written = fwrite(src, 1, size, (FILE*)_object);
    ASSERT( ferror((FILE*)_object) == 0, "fwrite failed: errno = %d", errno );
    return (size_t)bytes_written;
}

void File::flush()
{
    auto err = fflush((FILE*)_object);
    ASSERT( err == 0, "fwrite failed: errno = %d", errno );
}

////
bool Filesystem::initialize()
{
#ifdef PLATFORM_WIN32
    wchar_t path[kMaxPath];
    path[0] = 0;
    GetCurrentDirectory(kMaxPath, path);
    _working_directory.set(to_unix(path));
#else
    char path[kMaxPath];
    path[0] = 0;
    getcwd(path, kMaxPath);
    _working_directory.set(path);
#endif

    return true;
}

void Filesystem::dispose()
{

}

bool Filesystem::set_working_directory(const Path& path)
{
    auto nwd = path.is_absolute() ? path : _working_directory / path;
    auto string_path = to_string(nwd);

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(string_path.c_str());
    if( !SetCurrentDirectoryW(wstring_path.c_str()) )
#else
    if( chdir(string_path.c_str()) != 0 )
#endif
    {
        LOGW("failed to set working directory to \"%s\".", string_path.c_str());
        return false;
    }

    _working_directory = std::move(nwd);
    return true;
}

const Path& Filesystem::get_working_directory() const
{
    return _working_directory;
}

bool Filesystem::create_directory(const Path& path, bool recursive)
{
    auto parent = path.get_parent();
    if( !is_directory_exist(parent) )
    {
        if( !recursive )
        {
            LOGW("failed to create directory due to absence of parent directory");
            return false;
        }

        if( !create_directory(parent, recursive) )
            return false;
    }

    auto string_path = to_string(path.is_absolute() ? path : _working_directory / path);
#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(string_path.c_str());
    if( CreateDirectoryW(wstring_path.c_str(), nullptr) )
#else
    if( mkdir(string_path.c_str(), S_IRWXU) != 0 && errno != EEXIST )
#endif
    {
        LOGW("failed to create directory \"%s\".", string_path.c_str());
        return false;
    }

    return true;
}

bool Filesystem::move(const Path& from, const Path& to)
{
    auto from_str = to_string(from.is_absolute() ? from : _working_directory / from);
    auto to_str = to_string(to.is_absolute() ? to : _working_directory / to);

#ifdef PLATFORM_WIN32
    auto from_wstr = to_win32(from_str.c_str());
    auto to_wstr = to_win32(to_str.c_str());
    return MoveFileW(from_wstr.c_str(), to_wstr.c_str()) != 0;
#else
    return std::rename(from_str.c_str(), to_str.c_str()) == 0;
#endif
}

bool Filesystem::remove(const Path& path, bool recursive)
{
    auto string_path = to_string(path.is_absolute() ? path : _working_directory / path);

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(string_path.c_str());
    return DeleteFileW(wstring_path.c_str());
#else
    return std::remove(string_path.c_str()) == 0;
#endif
}

bool Filesystem::is_file_exist(const Path& path) const
{
    auto string_path = to_string(path.is_absolute() ? path : _working_directory / path);

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(string_path.c_str());
    DWORD attr = GetFileAttributesW(wstring_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY == 0);
#else
    struct stat sb;
    if( stat(string_path.c_str(), &sb) )
        return false;
    return S_ISREG(sb.st_mode);
#endif
}

bool Filesystem::is_directory_exist(const Path& path) const
{
    auto string_path = to_string(path.is_absolute() ? path : _working_directory / path);

#ifdef PLATFORM_WIN32
    auto wstring_path = to_win32(string_path.c_str());
    DWORD attr = GetFileAttributesW(wstring_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY != 0);
#else
    struct stat sb;
    if( stat(string_path.c_str(), &sb) )
        return false;
    return S_ISDIR(sb.st_mode);
#endif
}

std::unique_ptr<File> Filesystem::open(const Path& path, FileMode mode)
{
    auto file = new (std::nothrow) File(*this);
    if( file && file->open(path, mode) ) return std::unique_ptr<File>(file);
    if( file ) delete file;
    return nullptr;
}

NS_FLOW2D_RES_END
