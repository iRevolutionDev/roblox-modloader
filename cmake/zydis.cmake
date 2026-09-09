include(cmake/cpm.cmake)

if (NOT TARGET Zydis)
    CPMAddPackage(
            NAME Zydis
            GIT_REPOSITORY https://github.com/zyantific/zydis.git
            GIT_TAG v4.1.0
            OPTIONS
            "ZYDIS_BUILD_TOOLS OFF"
            "ZYDIS_BUILD_EXAMPLES OFF"
            "ZYDIS_BUILD_SHARED_LIB OFF"
    )
endif ()
