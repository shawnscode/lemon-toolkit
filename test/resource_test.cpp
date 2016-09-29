#include <catch.hpp>
#include <lemon-toolkit.hpp>
#include <codebase/debug/log.hpp>
#include <cstdio>

USING_NS_LEMON;
USING_NS_LEMON_CORE;
USING_NS_LEMON_RESOURCE;
USING_NS_LEMON_FILESYSTEM;

TEST_CASE("PathConstruct")
{
    REQUIRE( (Path {"/"}) == "/" );
    REQUIRE( (Path {"///"}) == "/" );

    REQUIRE( (Path {""}) == "./" );
    REQUIRE( (Path {"./"}) == "./" );
    REQUIRE( (Path {"./resource"}) == "resource" );
    REQUIRE( (Path {".////resource"}) == "resource" );
    REQUIRE( (Path {".////resource///image///"}) == "resource/image" );
    REQUIRE( (Path {"./resource/img./"}) == "resource/img." );
    REQUIRE( (Path {"./resource/.img"}) == "resource/.img" );
    REQUIRE( (Path {"././././/resource///image///"}) == "resource/image" );
    REQUIRE( (Path {"./resource/././/.//image////."}) == "resource/image" );
    REQUIRE( (Path {"./resource/././/.//image//.//./."}) == "resource/image" );
    REQUIRE( (Path {"/resource/././/.//image//.//./."}) == "/resource/image" );
    REQUIRE( (Path {".////resource///image///"}) == "resource/image" );

    REQUIRE( (Path {"/resource"}) == "/resource" );
    REQUIRE( (Path {"/resource/img"}) == "/resource/img" );
    REQUIRE( (Path {"/resource/img/.."}) == "/resource" );
    REQUIRE( (Path {"/resource/img/../"}) == "/resource" );
    REQUIRE( (Path {"/resource/img/..///"}) == "/resource" );
    REQUIRE( (Path {"/../../resource/img/.."}) == "/resource" );
    REQUIRE( (Path {"/../../resource/img/..//"}) == "/resource" );
    REQUIRE( (Path {"/resource/img/../..///../.."}) == "/" );
    REQUIRE( (Path {"/../../resource/img/../..///../.."}) == "/" );
    REQUIRE( (Path {"/resource///"}) == "/resource" );
    REQUIRE( (Path {"/resource///img"}) == "/resource/img" );
    REQUIRE( (Path {"/resource///img////"}) == "/resource/img" );

    REQUIRE( (Path {"//resource"}) == "/resource" );
    REQUIRE( (Path {"//resource/img"}) == "/resource/img" );
    REQUIRE( (Path {"//resource/img/../"}) == "/resource" );
    REQUIRE( (Path {"//resource/img/../../"}) == "/" );
    REQUIRE( (Path {"//resource/img/../..///"}) == "/" );
    REQUIRE( (Path {"//../../resource/img/.."}) == "/resource" );
    REQUIRE( (Path {"//../../resource/img/..//"}) == "/resource" );
    REQUIRE( (Path {"//resource/img/../..///../.."}) == "/" );
    REQUIRE( (Path {"//../../resource/img/../..///../.."}) == "/" );
    REQUIRE( (Path {"//resource///"}) == "/resource" );

    REQUIRE( (Path {"resource"}) == "resource" );
    REQUIRE( (Path {"resource/img"}) == "resource/img" );
    REQUIRE( (Path {"resource/img/../"}) == "resource" );
    REQUIRE( (Path {"resource/img/../../"}) == "./" );
    REQUIRE( (Path {"resource/img/../..///"}) == "./" );
    REQUIRE( (Path {"../resource/img/.."}) == "../resource" );
    REQUIRE( (Path {"../../resource/img/.."}) == "../../resource" );
    REQUIRE( (Path {"../../resource/img/..//"}) == "../../resource" );
    REQUIRE( (Path {"resource/img/../..///.."}) == ".." );
    REQUIRE( (Path {"resource/img/../..///../.."}) == "../.." );
    REQUIRE( (Path {"../../resource/img/../..///../.."}) == "../../../.." );
    REQUIRE( (Path {"resource//img"}) == "resource/img" );
    REQUIRE( (Path {"resource//img////"}) == "resource/img" );
    REQUIRE( (Path {"resource//img////pic"}) == "resource/img/pic" );
}

TEST_CASE("PathConcatenationAndOthers")
{
    Path path("/");
    Path path2("resource");
    Path path3("nocompress/pic.png");

    REQUIRE( (path / path2) == "/resource" );
    REQUIRE( ((path / path2) / path3) == "/resource/nocompress/pic.png" );

    REQUIRE( (path  / Path("")) == "/" );
    REQUIRE( (path2 / Path("")) == "resource" );
    REQUIRE( (path3 / Path("")) == "nocompress/pic.png" );

    REQUIRE( (Path("") / path2) == "resource" );
    REQUIRE( (Path("") / path3) == "nocompress/pic.png" );

    REQUIRE( (Path("") / path2 / Path("")) == "resource" );
    REQUIRE( (Path("") / path3 / Path("")) == "nocompress/pic.png" );

    Path path4("/resource/img/hero_head.png");
    REQUIRE( path4.get_filename() == "hero_head.png" );
    REQUIRE( path4.get_basename() == "hero_head" );
    REQUIRE( path4.get_extension() == "png" );

    REQUIRE( path4.get_parent() == (Path {"/resource/img"}) );
    REQUIRE( path4.get_parent().get_parent() == (Path {"/resource"}) );
    REQUIRE( path4.get_root() == (Path {"/"}) );
    REQUIRE( path4.get_root().get_root() == (Path {"/"}) );
    REQUIRE( Path("resource").get_parent() == "" );
    REQUIRE( Path("").get_parent() == ".." );
    REQUIRE( Path("").get_parent().get_parent() == "../.." );
    REQUIRE( Path("").get_parent().get_parent().get_parent() == "../../.." );
    REQUIRE( Path("resource/img").get_root() == "resource" );
    REQUIRE( Path("resource").get_root() == "resource" );

    Path::iterator iterator = path4.begin();
    REQUIRE( *iterator++ == "resource" );
    REQUIRE( *iterator++ == "img" );
    REQUIRE( *iterator++ == "hero_head.png" );
    REQUIRE( iterator == path4.end() );
}

TEST_CASE("TestFilesystem")
{
    auto pwd = get_current_directory();
    REQUIRE( set_current_directory("../../test") );
    remove("tmp2");
    remove("tmp", true);

    REQUIRE( create_directory("tmp2") );
    REQUIRE( is_directory("tmp2") );
    REQUIRE( !is_regular_file("tmp2") );

    REQUIRE( move("tmp2", "tmp") );
    REQUIRE( !is_directory("tmp2") );
    REQUIRE( is_directory("tmp") );

    auto file = open("tmp/resource.txt", FileMode::APPEND);
    file.write("hahaha", 6);
    file.close();

    remove("tmp2");
    remove("tmp", true);
    REQUIRE( !is_directory("tmp2") );
    REQUIRE( !is_directory("tmp") );
    REQUIRE( set_current_directory(pwd) );
}

struct ArchiveFixture
{
    ArchiveFixture()
    {
        initialize(0);

        set_output_stream(LogLevel::ERROR, &std::cout);
        pwd = get_current_directory();
        set_current_directory("../../test");
        add_subsystem<ArchiveCollection>();
    }

    ~ArchiveFixture()
    {
        set_current_directory(pwd);
        dispose();
    }

    Path pwd;
};

TEST_CASE_METHOD(ArchiveFixture, "TestArchiveCollection")
{
    remove("tmp", true);

    auto collection = get_subsystem<ArchiveCollection>();
    REQUIRE( !collection->add_search_path("tmp") );
    auto file = collection->open("resource.txt", FileMode::APPEND);
    REQUIRE( !file.is_open() );

    REQUIRE( create_directory("tmp") );
    REQUIRE( collection->add_search_path("tmp") );
    file = collection->open("resource.txt", FileMode::APPEND);
    REQUIRE( !file.is_open() );

    file = open("tmp/resource.txt", FileMode::APPEND);
    file.write("hahaha", 6);
    file.close();

    file = collection->open("resource.txt", FileMode::APPEND);
    REQUIRE( file.is_open() );
    file.close();

    remove("tmp", true);
}

struct Text : public Resource
{
    bool read(std::istream& stream) override
    {
        char buffer[512];
        memset(buffer, 0, sizeof(buffer));
        
        stream.read(buffer, 512);
        text = buffer;
        _memusage = 1024;
        return true;
    };

    bool save(std::ostream& stream) override
    {
        return true;
    }

    std::string text;
};

struct ResourceCacheFixture
{
    ResourceCacheFixture()
    {
        initialize(0);

        set_output_stream(LogLevel::ERROR, &std::cout);
        pwd = get_current_directory();
        set_current_directory("../../test");
        add_subsystem<ArchiveCollection>();
        add_subsystem<ResourceCache>(1025);

        REQUIRE( create_directory("tmp") );
        auto file = open("tmp/resource.txt", FileMode::APPEND);
        file.write("do", 2);
        file.close();

        file = open("tmp/resource2.txt", FileMode::APPEND);
        file.write("re", 2);
        file.close();

        file = open("tmp/resource3.txt", FileMode::APPEND);
        file.write("mi", 2);
        file.close();
    }

    ~ResourceCacheFixture()
    {
        remove("tmp", true);
        set_current_directory(pwd);

        dispose();
    }

    Path pwd;
};

TEST_CASE_METHOD(ResourceCacheFixture, "")
{
    auto cache = get_subsystem<ResourceCache>();
    auto collection = get_subsystem<ArchiveCollection>();

    auto r = cache->get<Text>("./resource.txt");
    REQUIRE( !r );

    collection->add_search_path("tmp");
    r = cache->get<Text>("./resource.txt");
    REQUIRE( r );

    REQUIRE( r->text == "do" );
    REQUIRE( r.use_count() == 3 );

    auto r2 = cache->get<Text>("./resource.txt");
    REQUIRE( r2 == r );
    REQUIRE( r2.use_count() == 4 );

    auto r3 = cache->get<Text>("./resource2.txt");
    REQUIRE( r3.use_count() == 3 );
    REQUIRE( cache->get_memusage() == 2048 );
    r3.reset();

    auto r4 = cache->get<Text>("./resource3.txt");
    REQUIRE( cache->get_memusage() == 2048 );

    r = cache->get<Text>("./resource.txt");
    REQUIRE( cache->get_memusage() == 2048 );

    r4 = cache->get<Text>("./resource2.txt");
    REQUIRE( cache->get_memusage() == 3072 );
};