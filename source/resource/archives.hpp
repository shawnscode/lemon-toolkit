// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/filesystem.hpp>

NS_LEMON_RESOURCE_BEGIN

struct Archive
{
    Archive(ArchiveCollection& collection) : _collection(collection) {}
    virtual ~Archive() {}

    virtual bool initialize() { return true; }
    virtual bool is_exist(const fs::Path&) = 0;
    virtual std::fstream open(const fs::Path&, fs::FileMode) = 0;

protected:
    ArchiveCollection& _collection;
};

struct FilesystemArchive : public Archive
{
    FilesystemArchive(ArchiveCollection&, const fs::Path&);

    bool initialize() override;
    bool is_exist(const fs::Path&) override;
    std::fstream open(const fs::Path&, fs::FileMode) override;

protected:
    fs::Path _prefix;
};

// stores files of a directory tree sequentially for convenient access
struct PackageArchive : public Archive
{
    PackageArchive(ArchiveCollection&, const fs::Path&, unsigned offset = 0);
    virtual ~PackageArchive();

    bool initialize() override;
    bool is_exist(const fs::Path&) override;
    std::fstream open(const fs::Path&, fs::FileMode) override;

protected:
    fs::Path        _filepath;
    unsigned        _offset;
    std::fstream    _stream;
};

//subsystem for file and archive operations and access control
struct ArchiveCollection : core::Subsystem
{
    SUBSYSTEM("ArchiveCollection");

    ArchiveCollection(core::Context& c) : Subsystem(c) {}
    virtual ~ArchiveCollection();

    template<typename T> using boolean = typename std::enable_if<
        std::is_base_of<Archive, T>::value,
        bool>::type;
    template<typename T, typename ... Args> boolean<T> add_archive(Args && ...);
    bool add_search_path(const fs::Path&);
    std::fstream open(const fs::Path&, fs::FileMode);

protected:
    std::vector<Archive*> _archives;
};

///
template<typename T, typename ... Args>
ArchiveCollection::boolean<T> ArchiveCollection::add_archive(Args&& ... args)
{
    auto arch = new (std::nothrow) T(*this, std::forward<Args>(args)...);
    if( arch && arch->initialize() )
    {
        _archives.push_back(arch);
        return true;
    }

    if( arch ) delete arch;
    return false;
}

NS_LEMON_RESOURCE_END