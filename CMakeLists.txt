cmake_minimum_required(VERSION 3.22.1)

project(roblox_modloader LANGUAGES CXX C ASM VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

# Directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(PUBLIC_HEADER_DIR "${PROJECT_SOURCE_DIR}/include")

# Fetch packages
message(STATUS "Fetching packages...")
include(scripts/spdlog.cmake)
include(scripts/minhook.cmake)
include(scripts/zlib.cmake)
include(scripts/toml.cmake)
include(scripts/tracy.cmake)
include(scripts/luau.cmake)

# Source files
message(STATUS "Roblox ModLoader")
file(GLOB_RECURSE SRC_MAIN
        "${SRC_DIR}/**.hpp"
        "${SRC_DIR}/**.h"
        "${SRC_DIR}/**.cpp"
        "${SRC_DIR}/**.cc"
        "${SRC_DIR}/**.cxx"
        "${SRC_DIR}/**.asm"
)

file(GLOB_RECURSE PUBLIC_HEADER_FILES
        "${PUBLIC_HEADER_DIR}/**.hpp"
)

# Libraries
add_library(${PROJECT_NAME} MODULE "${SRC_MAIN}" "${PUBLIC_HEADER_FILES}")
add_library(${PROJECT_NAME}_interface INTERFACE)

# Exclude main.cpp from interface target
file(GLOB_RECURSE SRC_MAIN_INTERFACE
        "${SRC_DIR}/**.hpp"
        "${SRC_DIR}/**.h"
        "${SRC_DIR}/**.cpp"
        "${SRC_DIR}/**.cc"
        "${SRC_DIR}/**.cxx"
        "${SRC_DIR}/**.asm"
)
list(FILTER SRC_MAIN_INTERFACE EXCLUDE REGEX "${SRC_DIR}/main.cpp")

# Compiler options
if (MSVC)
    add_compile_options(/bigobj)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else ()
    add_compile_options(-Wa,-mbig-obj)
    add_compile_options(-m32)
endif ()

set_target_properties(${PROJECT_NAME}_interface PROPERTIES LINKER_LANGUAGE CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 26)

source_group(TREE ${SRC_DIR} PREFIX "src" FILES ${SRC_MAIN})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        "${SRC_DIR}"
        "${PUBLIC_HEADER_DIR}"
        "${spdlog_SOURCE_DIR}"
        "${minhook_SOURCE_DIR}/include"
        "${zlib_SOURCE_DIR}"
        "${tomlplusplus_SOURCE_DIR}/include"
        "${tracy_SOURCE_DIR}/public"

        "${luau_SOURCE_DIR}/Compiler/include"
        "${luau_SOURCE_DIR}/Ast/include"
        "${luau_SOURCE_DIR}/VM/include"
        "${luau_SOURCE_DIR}/VM/src"
        "${luau_SOURCE_DIR}/EqSat/include"
        "${luau_SOURCE_DIR}/CodeGen/include"
        "${luau_SOURCE_DIR}/Common/include"
)

target_include_directories(${PROJECT_NAME}_interface INTERFACE
        "${PUBLIC_HEADER_DIR}"
        "${spdlog_SOURCE_DIR}"
        "${minhook_SOURCE_DIR}/include"
        "${zlib_SOURCE_DIR}"
        "${tomlplusplus_SOURCE_DIR}/include"
        "${tracy_SOURCE_DIR}/public"

        "${luau_SOURCE_DIR}/Compiler/include"
        "${luau_SOURCE_DIR}/Ast/include"
        "${luau_SOURCE_DIR}/VM/include"
        "${luau_SOURCE_DIR}/VM/src"
        "${luau_SOURCE_DIR}/EqSat/include"
        "${luau_SOURCE_DIR}/CodeGen/include"
        "${luau_SOURCE_DIR}/Common/include"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/RobloxModLoader/common.hpp")

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        spdlog
        minhook
        ZLIB::ZLIB
        Tracy::TracyClient

        Luau.Compiler
        Luau.Ast
        Luau.VM
        Luau.VM.Internals
        Luau.EqSat
        Luau.CodeGen
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_interface
        spdlog
        minhook
        ZLIB::ZLIB
        Tracy::TracyClient

        Luau.Compiler
        Luau.Ast
        Luau.VM
        Luau.VM.Internals
        Luau.EqSat
        Luau.CodeGen
)

# Add subdirectory
add_subdirectory(examples)

# Warning as error
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ROBLOX_MODLOADER_EXPORTS
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
)

target_compile_definitions(${PROJECT_NAME} PRIVATE IS_RML=1)

# Optimizations
if (MSVC)
    if (OPTIMIZE)
        set(CMAKE_MODULE_LINKER_FLAGS
                "${CMAKE_MODULE_LINKER_FLAGS} /LTCG /OPT:REF,ICF /GUARD:NO")

        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
                "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Oi /Ot /Oy /Ob3 /sdl- /GL /GF /GS- /Gw")
        string(REPLACE "/Ob1" "/Ob3" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    else ()
        add_compile_definitions(${PROJECT_NAME} "DEBUG")
    endif ()
endif ()

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif ()

# Ensure interface target compiles source files
target_sources(${PROJECT_NAME}_interface INTERFACE ${SRC_MAIN_INTERFACE})
