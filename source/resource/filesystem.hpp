// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/path.hpp>
#include <core/context.hpp>

NS_LEMON_FILESYSTEM_BEGIN

enum class FileMode : uint16_t
{
    READ        = 0x1,
    WRITE       = 0x2,
    // all output operations happen at the end of the file, appending to its existing contents
    APPEND      = 0x4,
    // operations are performed in binary mode rather than text
    BINARY      = 0x8,
    // the output position starts at the end of the file.
    ATEND       = 0x10,
    // any contents that existed in the file before it is open are discarded
    TRUNCATE    = 0x20
};

enum class ScanMode : uint16_t
{
    FILES       = 0x1,
    DIRECTORIES = 0x2,
    RECURSIVE   = 0x4,
    HIDDEN      = 0x8
};

// set the current working directory
bool set_current_directory(const Path&);
// get then current working directory
const Path& get_current_directory();

// create a directory
bool create_directory(const Path&, bool recursive = false);
// move a file, return true if successful
bool move(const Path&, const Path&);
// delete a file, optional recursive delete the contents of path if it exists,
// return true if successful
bool remove(const Path&, bool recursive = false);
// check if a file exists
bool is_exists(const Path&);
// check if a regular file exists
bool is_directory(const Path&);
// check if a directory exists
bool is_regular_file(const Path&);
// construct and open a filesystem file, return nullptr if failed
std::fstream open(const Path&, FileMode);

// Directory provides compliant iteration over the contents of a directory
struct Directory
{
    using iterator = std::vector<Path>::iterator;

    Directory() {}
    Directory(Directory&& rhs) { _nodes = std::move(rhs._nodes); }

    bool scan(const Path&, ScanMode);

    iterator begin() { return _nodes.begin(); }
    iterator end() { return _nodes.end(); }

protected:
    std::vector<Path> _nodes;
};

// construct and open a filesystem directory view
Directory scan(const Path&, ScanMode mode = ScanMode::FILES);

NS_LEMON_FILESYSTEM_END
ENABLE_BITMASK_OPERATORS(lemon::fs::FileMode);
ENABLE_BITMASK_OPERATORS(lemon::fs::ScanMode);