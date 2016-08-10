// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/filesystem.hpp>

NS_FLOW2D_RES_BEGIN

struct Archive
{
    virtual ~Archive() {}
    virtual bool is_exist(const fs::Path&) = 0;
    virtual std::fstream open(const fs::Path&, fs::FileMode) = 0;
};

struct FilesystemArchive : public Archive
{
    FilesystemArchive(const fs::Path&);

    bool is_exist(const fs::Path&) override;
    std::fstream open(const fs::Path&, fs::FileMode) override;

protected:
    fs::Path _prefix;
};

//subsystem for file and archive operations and access control
struct ArchiveCollection : core::Subsystem
{
    SUBSYSTEM("ArchiveCollection");

    ArchiveCollection(core::Context& c) : Subsystem(c) {}
    virtual ~ArchiveCollection() {}

    bool add_search_path(const fs::Path&);
    template<typename T, typename ... Args> bool add_archive(Args && ...);

    std::fstream open(const fs::Path&, fs::FileMode);

protected:
    std::vector<Archive*> _archives;
};

///
template<typename T, typename ... Args>
bool ArchiveCollection::add_archive(Args&& ... args)
{
    auto arch = new (std::nothrow) T(std::forward<Args>(args)...);
    if( arch ) _archives.push_back(arch);
    return arch != nullptr;
}

NS_FLOW2D_RES_END