cmake_minimum_required(VERSION 3.22.1)

project(RobloxModLoader LANGUAGES CXX C ASM ASM_MASM RC VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

option(ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR "Build the proxy generator tool" ON)
option(ROBLOX_MODLOADER_BUILD_PROXY_DLL "Build the dwmapi.dll proxy automatically" ON)
option(ROBLOX_MODLOADER_BUILD_EXAMPLES "Build example mods" ON)
option(ROBLOX_MODLOADER_INSTALL "Install RobloxModLoader" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ROBLOX_MODLOADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ROBLOX_MODLOADER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include(cmake/cpm.cmake)

# Core dependencies
include(scripts/spdlog.cmake)
include(scripts/minhook.cmake)
include(scripts/zlib.cmake)
include(scripts/toml.cmake)
include(scripts/tracy.cmake)
include(scripts/luau.cmake)
include(scripts/json.cmake)

include(cmake/dependencies.cmake)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(MODLOADER_SOURCES
            "${ROBLOX_MODLOADER_SOURCE_DIR}/main.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/mod_manager.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/pointers.cpp"
    )

    file(GLOB_RECURSE MODLOADER_SUBDIRS CONFIGURE_DEPENDS
            "${ROBLOX_MODLOADER_SOURCE_DIR}/config/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/exception/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/hooking/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/hooks/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/logger/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/luau/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/memory/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/mod/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/roblox/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/utils/*.cpp"
    )

    list(APPEND MODLOADER_SOURCES ${MODLOADER_SUBDIRS})

    add_library(roblox_modloader SHARED ${MODLOADER_SOURCES})

    setup_compiler_flags(roblox_modloader)
    setup_compile_definitions(roblox_modloader PRIVATE)
    setup_core_dependencies(roblox_modloader PRIVATE)
    setup_luau_dependencies(roblox_modloader PRIVATE)

    target_include_directories(roblox_modloader PRIVATE
            "${ROBLOX_MODLOADER_SOURCE_DIR}"
            "${ROBLOX_MODLOADER_INCLUDE_DIR}"
    )

    set_target_properties(roblox_modloader PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            FOLDER "ModLoader"
            COMPILE_WARNING_AS_ERROR ON
            ENABLE_EXPORTS ON
    )

    target_precompile_headers(roblox_modloader PRIVATE
            "${ROBLOX_MODLOADER_INCLUDE_DIR}/RobloxModLoader/common.hpp"
    )

    include(GenerateExportHeader)
    generate_export_header(roblox_modloader
            EXPORT_FILE_NAME "${ROBLOX_MODLOADER_INCLUDE_DIR}/RobloxModLoader/rml_export.hpp"
            EXPORT_MACRO_NAME "RML_EXPORT"
            NO_EXPORT_MACRO_NAME "RML_NO_EXPORT"
            DEPRECATED_MACRO_NAME "RML_DEPRECATED"
    )
endif ()

add_library(RobloxModLoader INTERFACE)
add_library(RobloxModLoader::RobloxModLoader ALIAS RobloxModLoader)

target_include_directories(RobloxModLoader INTERFACE
        "$<BUILD_INTERFACE:${ROBLOX_MODLOADER_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
        # Include all dependency headers that common.hpp needs
        "$<BUILD_INTERFACE:${spdlog_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${tomlplusplus_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${tracy_SOURCE_DIR}/public>"
        "$<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${minhook_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${luau_SOURCE_DIR}/Compiler/include>"
        "$<BUILD_INTERFACE:${luau_SOURCE_DIR}/Ast/include>"
        "$<BUILD_INTERFACE:${luau_SOURCE_DIR}/VM/include>"
        "$<BUILD_INTERFACE:${luau_SOURCE_DIR}/Common/include>"
)

target_link_libraries(RobloxModLoader INTERFACE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
        Tracy::TracyClient
        minhook
        Luau.Compiler
        Luau.Ast
        Luau.VM
        Luau.VM.Internals
        Luau.EqSat
        Luau.CodeGen
        $<$<TARGET_EXISTS:roblox_modloader>:roblox_modloader>
)

target_compile_features(RobloxModLoader INTERFACE cxx_std_23)

target_compile_definitions(RobloxModLoader INTERFACE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        IS_RML=1
        TOML_EXCEPTIONS=0
        $<$<CONFIG:Debug>:DEBUG>
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR)
        add_subdirectory(proxy_generator)
    endif ()

    if (ROBLOX_MODLOADER_BUILD_PROXY_DLL AND ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR)
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

        add_dependencies(roblox_modloader dwmapi_proxy)

        target_link_libraries(dwmapi_proxy PRIVATE user32 kernel32)
        target_compile_definitions(dwmapi_proxy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

        message(STATUS "dwmapi.dll proxy will be built automatically with the main project")
        message(STATUS "Proxy DLL will be output to: ${CMAKE_BINARY_DIR}/${DLL_NAME}.dll")
    endif ()

    if (ROBLOX_MODLOADER_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()
endif ()

if (ROBLOX_MODLOADER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY "${ROBLOX_MODLOADER_INCLUDE_DIR}/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    )

    install(TARGETS RobloxModLoader
            EXPORT RobloxModLoaderTargets
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RobloxModLoaderConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )

    install(EXPORT RobloxModLoaderTargets
            FILE RobloxModLoaderTargets.cmake
            NAMESPACE RobloxModLoader::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )
endif ()