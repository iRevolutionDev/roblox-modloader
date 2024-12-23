cmake_minimum_required(VERSION 3.26)

project(roblox-modloader LANGUAGES CXX C ASM VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(PUBLIC_HEADER_DIR "${PROJECT_SOURCE_DIR}/include")

message(STATUS "Fetching packages...")
include(scripts/spdlog.cmake)

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

if (MSVC)
    add_compile_options(/bigobj)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else ()
    add_compile_options(-Wa,-mbig-obj)
    add_compile_options(-m32)
endif ()

add_library(${PROJECT_NAME} MODULE "${SRC_MAIN}" "${PUBLIC_HEADER_FILES}")

add_library(${PROJECT_NAME}_interface INTERFACE)
target_include_directories(${PROJECT_NAME}_interface INTERFACE "${PUBLIC_HEADER_DIR}")
target_link_libraries(${PROJECT_NAME}_interface INTERFACE spdlog)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

source_group(TREE ${SRC_DIR} PREFIX "src" FILES ${SRC_MAIN})

target_include_directories(${PROJECT_NAME} PRIVATE
        "${SRC_DIR}"
        "${PUBLIC_HEADER_DIR}"
        "${spdlog_SOURCE_DIR}"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SRC_DIR}/common.hpp")
target_link_libraries(${PROJECT_NAME} PRIVATE
        spdlog
)

add_subdirectory(examples)

# Warning as error
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

add_compile_definitions(${PROJECT_NAME}
        "_CRT_SECURE_NO_WARNINGS"
        "NOMINMAX"
        "WIN32_LEAN_AND_MEAN"
)
# Optimizations
if (MSVC)
    if (OPTIMIZE)
        set(CMAKE_MODULE_LINKER_FLAGS
                "${CMAKE_MODULE_LINKER_FLAGS} /LTCG /OPT:REF,ICF /GUARD:NO")

        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
                "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Oi /Ot /Oy /Ob3 /sdl- /GL /GF /GS- /Gw")
        string(REPLACE "/Ob1" "/Ob3" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    else ()
        add_compile_definitions(YIM_DEV)
    endif ()
endif ()

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif ()

set(ROBLOX_VERSION "version-281ccda49d704f6e")
set(USERNAME "Revolution")

#Copy the DLL to the Roblox directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "C:/Users/${USERNAME}/AppData/Local/Roblox/Versions/${ROBLOX_VERSION}/RobloxModLoader"
        COMMENT "Copying the DLL to the Roblox directory"
)