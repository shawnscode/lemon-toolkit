#include <catch.hpp>
#include <flow2d.hpp>
#include <cstdio>

USING_NS_FLOW2D;

struct Text : public ResourceTrait<Text>
{
    bool load_from_file(DataStream::Ptr stream) override
    {
        char buffer[512];
        stream->read(buffer, 512);
        text = buffer;
        return true;
    };

    size_t get_memory_usage() const override
    {
        return 1023;
    }

    std::string text;
};

struct ResourceCacheFixture
{
    ResourceCacheFixture() : cache(archives) {}
    ArchiveManager          archives;
    ResourceCacheManager    cache;
};


TEST_CASE("Path")
{
    Path path { "/" };
    REQUIRE( path.is_empty() );

    Path path2 { "/resource" };
    REQUIRE( !path2.is_empty() );
    REQUIRE( (path / path2) == path2 );

    Path path3 { "/resource/img/hero_head.png" };
    REQUIRE( path3.get_filename() == "hero_head.png" );
    REQUIRE( path3.get_basename() == "hero_head" );
    REQUIRE( path3.get_extension() == "png" );

    REQUIRE( path3.get_parent() == (Path {"/resource/img"}) );
    REQUIRE( path3.get_parent().get_parent() == (Path {"/resource"}) );

    REQUIRE( (*path3.begin()) == "resource" );
}

TEST_CASE_METHOD(ArchiveManager, "FilesystemArchive")
{
    auto s1 = open("resource.txt");
    REQUIRE( !s1 );

    add_archive<FilesystemArchive>();
    auto s2 = open("resource.txt");
    REQUIRE( s2 );
}

TEST_CASE_METHOD(ResourceCacheFixture, "")
{
    auto r = cache.get<Text>("./resource.txt");
    REQUIRE( !r );

    archives.add_archive<FilesystemArchive>();
    r = cache.get<Text>("./resource.txt");
    REQUIRE( r );

    REQUIRE( r->text == "hahaha" );
    REQUIRE( r.get_refcount() == 1 );

    r.retain();
    REQUIRE( r.get_refcount() == 2 );

    auto r2 = cache.get<Text>("./resource.txt");
    REQUIRE( r2 == r );
    REQUIRE( r2.get_refcount() == 2 );

    r2.retain();
    REQUIRE( r2.get_refcount() == 3 );

    auto r3 = cache.get<Text>("./resource2.txt");
    REQUIRE( r3.get_refcount() == 1 );

    r3.retain();

    REQUIRE( cache.get_memory_usage() == 2046 );

    r.release();
    r2.release();

    auto r4 = cache.get<Text>("./resource3.txt");
    REQUIRE( cache.get_memory_usage() == 2046 );

    r = cache.get<Text>("./resource.txt");
    REQUIRE( cache.get_memory_usage() == 2046 );
    r.retain();

    r4 = cache.get<Text>("./resource3.txt");
    REQUIRE( cache.get_memory_usage() == 3069 );
};