include(cmake/cpm.cmake)

if (NOT TARGET capstone)
    CPMAddPackage(
            NAME capstone
            GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
            GIT_TAG 5.0.9
            OPTIONS
            "BUILD_SHARED_LIBS OFF"
            "BUILD_STATIC_LIBS ON"
            "CAPSTONE_BUILD_CSTOOL OFF"
            "CAPSTONE_BUILD_CSTEST OFF"
            "CAPSTONE_BUILD_TESTS OFF"
            "CAPSTONE_INSTALL OFF"
            "CAPSTONE_ARCHITECTURE_DEFAULT OFF"
            "CAPSTONE_ARM64_SUPPORT ON"
    )
endif ()
