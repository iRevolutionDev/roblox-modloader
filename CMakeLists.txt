cmake_minimum_required(VERSION 3.22.1)

project(roblox_modloader LANGUAGES CXX C ASM ASM_MASM RC VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

option(BUILD_PROXY_GENERATOR "Build the proxy generator tool" ON)
option(BUILD_PROXY_DLL "Build the dwmapi.dll proxy automatically" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(PUBLIC_HEADER_DIR "${PROJECT_SOURCE_DIR}/include")

include(cmake/dependencies.cmake)
message(STATUS "Fetching packages...")
include(scripts/spdlog.cmake)
include(scripts/minhook.cmake)
include(scripts/zlib.cmake)
include(scripts/toml.cmake)
include(scripts/tracy.cmake)
include(scripts/luau.cmake)

set(CORE_SOURCES
        "${SRC_DIR}/mod_manager.cpp"
        "${SRC_DIR}/pointers.cpp"
)

file(GLOB_RECURSE CORE_SUBDIRS CONFIGURE_DEPENDS
        "${SRC_DIR}/config/*.cpp"
        "${SRC_DIR}/exception/*.cpp"
        "${SRC_DIR}/hooking/*.cpp"
        "${SRC_DIR}/hooks/*.cpp"
        "${SRC_DIR}/logger/*.cpp"
        "${SRC_DIR}/luau/*.cpp"
        "${SRC_DIR}/memory/*.cpp"
        "${SRC_DIR}/mod/*.cpp"
        "${SRC_DIR}/roblox/*.cpp"
        "${SRC_DIR}/utils/*.cpp"
)

list(APPEND CORE_SOURCES ${CORE_SUBDIRS})

file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS
        "${PUBLIC_HEADER_DIR}/**.hpp"
        "${PUBLIC_HEADER_DIR}/**.h"
)

add_library(${PROJECT_NAME}_core STATIC ${CORE_SOURCES} ${PUBLIC_HEADERS})

add_library(${PROJECT_NAME} MODULE "${SRC_DIR}/main.cpp")

add_library(${PROJECT_NAME}_interface INTERFACE)

setup_compiler_flags(${PROJECT_NAME}_core)
setup_compile_definitions(${PROJECT_NAME}_core PUBLIC)
setup_core_dependencies(${PROJECT_NAME}_core PUBLIC)
setup_luau_dependencies(${PROJECT_NAME}_core PUBLIC)

target_compile_features(${PROJECT_NAME}_core PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME}_core PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        FOLDER "Core"
)

setup_compiler_flags(${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE ROBLOX_MODLOADER_EXPORTS)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "Core"
)

source_group(TREE ${SRC_DIR} PREFIX "src" FILES ${CORE_SOURCES})
source_group(TREE ${PUBLIC_HEADER_DIR} PREFIX "include" FILES ${PUBLIC_HEADERS})

target_include_directories(${PROJECT_NAME}_core
        PUBLIC
        "$<BUILD_INTERFACE:${PUBLIC_HEADER_DIR}>"
        "$<INSTALL_INTERFACE:include>"
        PRIVATE
        "${SRC_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${SRC_DIR}"
        "${PUBLIC_HEADER_DIR}"
)

target_include_directories(${PROJECT_NAME}_interface INTERFACE
        "$<BUILD_INTERFACE:${PUBLIC_HEADER_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

target_precompile_headers(${PROJECT_NAME}_core PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/RobloxModLoader/common.hpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}_core
)

target_link_libraries(${PROJECT_NAME}_interface INTERFACE
        ${PROJECT_NAME}_core
)

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_property(TARGET ${PROJECT_NAME}_core PROPERTY COMPILE_WARNING_AS_ERROR ON)

set_target_properties(${PROJECT_NAME}_interface PROPERTIES FOLDER "Core")

if (BUILD_PROXY_GENERATOR)
    add_subdirectory(proxy_generator)
endif ()

if (BUILD_PROXY_DLL AND BUILD_PROXY_GENERATOR)
    set(PROXY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/proxy_generated")

    get_filename_component(DLL_NAME "C:/Windows/System32/dwmapi.dll" NAME_WE)

    add_custom_target(generate_dwmapi_proxy
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROXY_OUTPUT_DIR}
            COMMAND $<TARGET_FILE:proxy_generator> "C:/Windows/System32/dwmapi.dll" ${PROXY_OUTPUT_DIR}
            DEPENDS proxy_generator
            COMMENT "Generating dwmapi.dll proxy files"
            VERBATIM
    )

    add_custom_command(
            OUTPUT ${PROXY_OUTPUT_DIR}/dllmain.cpp ${PROXY_OUTPUT_DIR}/${DLL_NAME}.def ${PROXY_OUTPUT_DIR}/${DLL_NAME}.asm ${PROXY_OUTPUT_DIR}/proxy.rc
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROXY_OUTPUT_DIR}
            COMMAND $<TARGET_FILE:proxy_generator> "C:/Windows/System32/dwmapi.dll" ${PROXY_OUTPUT_DIR}
            DEPENDS proxy_generator
            COMMENT "Generating dwmapi.dll proxy source files"
            VERBATIM
    )

    add_library(dwmapi_proxy SHARED
            ${PROXY_OUTPUT_DIR}/dllmain.cpp
            ${PROXY_OUTPUT_DIR}/${DLL_NAME}.def
            ${PROXY_OUTPUT_DIR}/${DLL_NAME}.asm
            ${PROXY_OUTPUT_DIR}/proxy.rc
    )

    enable_language(RC)

    set_target_properties(dwmapi_proxy PROPERTIES
            OUTPUT_NAME ${DLL_NAME}
            SUFFIX ".dll"
            LINK_FLAGS "/DEF:${PROXY_OUTPUT_DIR}/${DLL_NAME}.def"
            FOLDER "Proxy"
    )

    add_dependencies(${PROJECT_NAME} dwmapi_proxy)

    target_link_libraries(dwmapi_proxy PRIVATE
            user32
            kernel32
    )

    target_compile_definitions(dwmapi_proxy PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )

    message(STATUS "dwmapi.dll proxy will be built automatically with the main project")
    message(STATUS "Proxy DLL will be output to: ${CMAKE_BINARY_DIR}/${DLL_NAME}.dll")
endif ()

add_subdirectory(examples)
