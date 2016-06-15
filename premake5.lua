workspace("flow2d")
    configurations({ "debug", "release" })
    location( "build" )

    filter("configurations:debug")
        defines({ "DEBUG" })
        flags({ "Symbols" })

    filter("configurations:release")
        defines({ "NDEBUG" })
        optimize( "On" )

    project("flow2d")
        kind( "StaticLib" )
        language( "C++" )
        buildoptions({"-std=c++11", "-stdlib=libc++"})
        files({ "source/**.cpp" })
        sysincludedirs({ "source", "/usr/local/include", "3rd/filesystem" })
        targetdir( "build/libs" )

workspace("chore")
    configurations( "debug" )
    defines({ "DEBUG" })
    flags({ "Symbols" })
    kind( "ConsoleApp" )
    libdirs({ "build/libs", "/usr/local/lib/" })

    sysincludedirs({ "source", "3rd/catch/include", "3rd/filesystem", "/usr/local/include" })
    includedirs({ "source", "3rd/catch/include", "3rd/filesystem", "/usr/local/include" })

    language( "C++" )
    buildoptions({"-std=c++11", "-stdlib=libc++"})
    targetdir( "build/bin" )

    project( "test" )
        location( "build/test" )
        links({ "flow2d" })
        files({ "test/*_test.cpp" })

    project( "example" )
        location( "build/example" )
        links({ "glfw3", "glew", "flow2d" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "example/*.cpp" })