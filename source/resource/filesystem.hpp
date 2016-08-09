// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/path.hpp>
#include <core/context.hpp>

NS_FLOW2D_RES_BEGIN

enum class FileMode : uint8_t
{
    READ    = 0x1,
    WRITE   = 0x2,
    APPEND  = 0x4,
    BINARY  = 0x8
};

struct File
{
    ~File() { close(); }

    // open a filesystem file, return true if successful
    bool open(const Path&, FileMode);
    // close the file
    void close();

    // read bytes from the file, return number of bytes actually read
    unsigned read(void*, unsigned);
    // set position from the begining of the file
    void seek(unsigned);
    // check if current position reach end of the file
    bool is_end() const;
    // write bytes to the file, might be buffered, return number of bytes actually written.
    unsigned write(const void*, unsigned);
    // flush any buffered output to the file
    void flush();

    // return a checksum of the file contents using SDBM hash algorithm
    unsigned get_checksum() const;

protected:
    friend class Filesystem;

    // file could only be opened through the filesystem
    File(Filesystem& fs) : _filesystem(fs) {}

    Filesystem& _filesystem;
    FileMode    _mode;
    void*       _object;
};

// subsystem for file and directory operations and access control
struct Filesystem : core::Subsystem
{
    SUBSYSTEM("Filesystem");

    Filesystem(core::Context& c) : Subsystem(c) {}
    virtual ~Filesystem() {}

    bool initialize() override;
    void dispose() override;

    // set the current working directory
    bool set_working_directory(const Path&);
    // get then current working directory
    const Path& get_working_directory() const;

    // create a directory
    bool create_directory(const Path&, bool recursive = false);
    // move a file, return true if successful
    bool move(const Path&, const Path&);
    // delete a file, optional recursive delete the contents of path if it exists,
    // return true if successful
    bool remove(const Path&, bool recursive = false);
    // check if a file exists
    bool is_file_exist(const Path&) const;
    // check if a directory exists
    bool is_directory_exist(const Path&) const;
    // construct and open a filesystem file, return nullptr if failed
    std::unique_ptr<File> open(const Path&, FileMode);
    // construct and open a filesystem directory, return nullptr if failed
    // std::unique_ptr<Directory> open(const Path&);

protected:
    Path _working_directory;
};

NS_FLOW2D_RES_END
ENABLE_BITMASK_OPERATORS(flow2d::res::FileMode);