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
        includedirs({ "source" })
        targetdir( "libs" )
