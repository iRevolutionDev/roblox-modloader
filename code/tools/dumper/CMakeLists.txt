cmake_minimum_required(VERSION 3.25)

# Use Zydis from PolyHook2 if available, otherwise fetch it
if(NOT TARGET Zydis)
    include(FetchContent)
    FetchContent_Declare(
        Zydis
        GIT_REPOSITORY https://github.com/zyantific/zydis.git
        GIT_TAG v4.1.0
    )
    FetchContent_MakeAvailable(Zydis)
endif()

file(GLOB_RECURSE DUMPER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(dumper ${DUMPER_SOURCES})

target_compile_features(dumper PRIVATE cxx_std_23)

target_link_libraries(dumper PRIVATE
        spdlog::spdlog
        imagehlp
        Zydis
)

target_compile_definitions(dumper PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
)

set_target_properties(dumper PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "dumper"
        FOLDER "Tools"
)

target_include_directories(dumper PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_precompile_headers(dumper PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/dumper/common.hpp"
)
