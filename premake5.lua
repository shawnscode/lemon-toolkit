workspace("lemon-toolkit")
    configurations({ "debug", "release" })
    location( "build" )

    filter("configurations:debug")
        defines({ "DEBUG" })
        flags({ "Symbols" })

    filter("configurations:release")
        defines({ "NDEBUG" })
        optimize( "On" )

    project("lemon-toolkit")
        kind( "StaticLib" )
        language( "C++" )
        buildoptions({"-std=c++11", "-stdlib=libc++"})
        files({ "source/**.cpp" })
        sysincludedirs({ "source", "/usr/local/include", "3rd/filesystem", "3rd/libtess2/Include" })
        targetdir( "build/libs" )

workspace( "dependencies" )
configurations( "release" )
defines({ "NDEBUG" })
optimize( "On" )
location( "build/3rd" )

project("tess2")
    kind( "StaticLib" )
    language( "C" )
    includedirs({ "3rd/libtess2/Include", "3rd/libtess2/Source" })
    files({ "3rd/libtess2/Source/*.c" })
    targetdir( "build/libs" )

workspace("chore")
    configurations( "debug" )
    defines({ "DEBUG" })
    flags({ "Symbols" })
    kind( "ConsoleApp" )
    libdirs({ "/usr/local/lib/" })

    sysincludedirs({ "source", "3rd/catch/include", "/usr/local/include", "3rd/libtess2/Include", "3rd/stb", "3rd/hayai/src" })
    includedirs({ "source", "3rd/catch/include", "/usr/local/include", "3rd/libtess2/Include", "3rd/stb", "3rd/hayai/src" })

    language( "C++" )
    buildoptions({"-std=c++11", "-stdlib=libc++"})
    targetdir( "build/bin" )

    project( "test" )
        location( "build/test" )
        links({ "glew", "SDL2" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "test/*_test.cpp", "source/**.cpp" })

    project( "phong" )
        location( "build/example" )
        links({ "glew", "SDL2" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "example/phong/*.cpp", "source/**.cpp" })
