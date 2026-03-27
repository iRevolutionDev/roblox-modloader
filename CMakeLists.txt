cmake_minimum_required(VERSION 3.22.1)

if (WIN32)
    set(RML_DEFAULT_BUILD_PROXY_GENERATOR ON)
    set(RML_DEFAULT_BUILD_PROXY_DLL ON)
    set(RML_DEFAULT_BUILD_MANAGED_PROJECTS ON)
    set(RML_DEFAULT_USE_CMAKE_CSHARP ON)
    set(ROBLOX_MODLOADER_ENABLE_MANAGED_TOOLING ON)
else ()
    set(RML_DEFAULT_BUILD_PROXY_GENERATOR OFF)
    set(RML_DEFAULT_BUILD_PROXY_DLL OFF)
    set(RML_DEFAULT_BUILD_MANAGED_PROJECTS ON)
    set(RML_DEFAULT_USE_CMAKE_CSHARP OFF)
endif ()

project(RobloxModLoader LANGUAGES CXX C ASM VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

if (WIN32)
    enable_language(ASM_MASM)
    enable_language(RC)
endif ()

option(ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR "Build the proxy generator tool" ${RML_DEFAULT_BUILD_PROXY_GENERATOR})
option(ROBLOX_MODLOADER_BUILD_PROXY_DLL "Build the dwmapi.dll proxy automatically" ${RML_DEFAULT_BUILD_PROXY_DLL})
option(ROBLOX_MODLOADER_BUILD_DUMPER "Build the Luau structures dumper tool" ON)
option(ROBLOX_MODLOADER_BUILD_EXAMPLES "Build example mods" ON)
option(ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS "Build managed C# projects as part of the default ALL target" ${RML_DEFAULT_BUILD_MANAGED_PROJECTS})
option(ROBLOX_MODLOADER_USE_CMAKE_CSHARP "Use CMake's native C# project integration on Visual Studio generators" ${RML_DEFAULT_USE_CMAKE_CSHARP})
option(ROBLOX_MODLOADER_INSTALL "Install RobloxModLoader" OFF)
option(ROBLOX_MODLOADER_ENABLE_MANAGED_TOOLING "Enable optional managed solution/tooling targets" OFF)

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
#include(scripts/tracy.cmake)
include(scripts/luau.cmake)
include(scripts/json.cmake)
include(scripts/polyhooks.cmake)
include(scripts/dotnet_hosting.cmake)

include(cmake/dependencies.cmake)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(MODLOADER_SOURCES
            "${ROBLOX_MODLOADER_SOURCE_DIR}/main.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/mod_manager.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/pointers.cpp"
    )

    file(GLOB_RECURSE MODLOADER_SUBDIRS CONFIGURE_DEPENDS
            "${ROBLOX_MODLOADER_SOURCE_DIR}/config/*.cpp"
            "${ROBLOX_MODLOADER_SOURCE_DIR}/dotnet/*.cpp"
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
    setup_dotnet_hosting(roblox_modloader PRIVATE)

    if (UNIX AND NOT APPLE)
        target_link_libraries(roblox_modloader PRIVATE dl)
    endif ()

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
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
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
        #"$<BUILD_INTERFACE:${tracy_SOURCE_DIR}/public>"
        "$<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${minhook_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Compiler/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Ast/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/VM/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Common/include>"
        "$<BUILD_INTERFACE:${polyhook2_SOURCE_DIR}>"
)

target_link_libraries(RobloxModLoader INTERFACE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
        #Tracy::TracyClient
        minhook
        PolyHook_2
        Luau.Compiler
        Luau.Ast
        Luau.VM
        Luau.VM.Internals
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

setup_dotnet_hosting(RobloxModLoader INTERFACE)

if (WIN32 AND TARGET roblox_modloader AND DEFINED ENV{ROBLOX_STUDIO_PATH} AND NOT "$ENV{ROBLOX_STUDIO_PATH}" STREQUAL "")
    add_custom_command(TARGET roblox_modloader POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{ROBLOX_STUDIO_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:roblox_modloader>
            "$ENV{ROBLOX_STUDIO_PATH}/$<TARGET_FILE_NAME:roblox_modloader>"
            COMMENT "Copying roblox_modloader.dll to $ENV{ROBLOX_STUDIO_PATH}"
    )

    if (RML_DOTNET_RUNTIME_ROOT AND EXISTS "${RML_DOTNET_RUNTIME_ROOT}")
        add_custom_command(TARGET roblox_modloader POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${RML_DOTNET_RUNTIME_ROOT}"
                "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime/dotnet"
                COMMENT "Copying bundled .NET runtime to RobloxModLoader/runtime/dotnet"
        )
    endif ()

    if (RML_DOTNET_GLOBAL_RUNTIME_CONFIG AND EXISTS "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}")
        add_custom_command(TARGET roblox_modloader POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}"
                "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime/RobloxModLoader.runtimeconfig.json"
                COMMENT "Copying global .NET runtimeconfig to RobloxModLoader/runtime"
        )
    endif ()

    if (RML_DOTNET_NETHOST_RUNTIME_BINARY AND EXISTS "${RML_DOTNET_NETHOST_RUNTIME_BINARY}")
        get_filename_component(RML_NETHOST_RUNTIME_NAME "${RML_DOTNET_NETHOST_RUNTIME_BINARY}" NAME)
        add_custom_command(TARGET roblox_modloader POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RML_DOTNET_NETHOST_RUNTIME_BINARY}"
                "$ENV{ROBLOX_STUDIO_PATH}/RobloxModLoader/runtime/${RML_NETHOST_RUNTIME_NAME}"
                COMMENT "Copying nethost runtime binary to RobloxModLoader/runtime"
        )
    endif ()
endif ()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR)
        if (WIN32)
            add_subdirectory(proxy_generator)
        else ()
            message(STATUS "Skipping proxy_generator: only supported on Windows")
        endif ()
    endif ()

    if (ROBLOX_MODLOADER_BUILD_DUMPER)
        add_subdirectory(dumper)
    endif ()

    if (ROBLOX_MODLOADER_BUILD_PROXY_DLL AND ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR)
        if (WIN32)
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
        else ()
            message(STATUS "Skipping proxy DLL generation: only supported on Windows")
        endif ()
    endif ()

    if (ROBLOX_MODLOADER_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()

    if (ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS OR ROBLOX_MODLOADER_ENABLE_MANAGED_TOOLING)
        set(RML_MANAGED_PROJECTS
                "${CMAKE_CURRENT_SOURCE_DIR}/managed/RobloxModLoader.Managed/RobloxModLoader.Managed.csproj"
        )

        set(RML_USE_NATIVE_CMAKE_CSHARP OFF)
        if (ROBLOX_MODLOADER_USE_CMAKE_CSHARP AND WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
            set(RML_USE_NATIVE_CMAKE_CSHARP ON)
        endif ()

        if (RML_USE_NATIVE_CMAKE_CSHARP)
            if (ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS)
                add_custom_target(rml_managed_build_all ALL
                        COMMENT "Building all managed projects through Visual Studio/CMake C# integration"
                )
                set_target_properties(rml_managed_build_all PROPERTIES FOLDER "Managed")

                include_external_msproject(
                        rml_managed_build_sdk
                        "${CMAKE_CURRENT_SOURCE_DIR}/managed/RobloxModLoader.Managed/RobloxModLoader.Managed.csproj"
                )

                set_target_properties(rml_managed_build_sdk PROPERTIES FOLDER "Managed")

                add_dependencies(rml_managed_build_all
                        rml_managed_build_sdk
                )

                if (TARGET roblox_modloader)
                    add_dependencies(roblox_modloader rml_managed_build_all)
                endif ()
            endif ()
        else ()
            find_program(DOTNET_EXECUTABLE dotnet REQUIRED)

            if (CMAKE_CONFIGURATION_TYPES)
                set(RML_DOTNET_CONFIGURATION "$<CONFIG>")
            elseif (CMAKE_BUILD_TYPE)
                set(RML_DOTNET_CONFIGURATION "${CMAKE_BUILD_TYPE}")
            else ()
                set(RML_DOTNET_CONFIGURATION "Release")
            endif ()

            add_custom_target(rml_managed_restore
                    COMMAND "${DOTNET_EXECUTABLE}" restore "${CMAKE_CURRENT_SOURCE_DIR}/managed/RobloxModLoader.Managed/RobloxModLoader.Managed.csproj"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Restoring NuGet packages for managed projects"
                    VERBATIM
            )
            set_target_properties(rml_managed_restore PROPERTIES FOLDER "Managed")

            function(rml_add_managed_build_target target_name project_path project_label)
                add_custom_target(${target_name}
                        COMMAND "${DOTNET_EXECUTABLE}" build "${project_path}" -c "${RML_DOTNET_CONFIGURATION}" --no-restore
                        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                        DEPENDS rml_managed_restore
                        COMMENT "Building ${project_label} (${RML_DOTNET_CONFIGURATION})"
                        VERBATIM
                )
                set_target_properties(${target_name} PROPERTIES FOLDER "Managed")
            endfunction()

            rml_add_managed_build_target(
                    rml_managed_build_sdk
                    "${CMAKE_CURRENT_SOURCE_DIR}/managed/RobloxModLoader.Managed/RobloxModLoader.Managed.csproj"
                    "RobloxModLoader.Managed"
            )

            if (ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS)
                add_custom_target(rml_managed_build_all ALL
                        COMMENT "Building all managed projects"
                )
                set_target_properties(rml_managed_build_all PROPERTIES FOLDER "Managed")

                add_dependencies(rml_managed_build_all
                        rml_managed_build_sdk
                )

                if (TARGET roblox_modloader)
                    add_dependencies(roblox_modloader rml_managed_build_all)
                endif ()
            endif ()
        endif ()

        if (ROBLOX_MODLOADER_ENABLE_MANAGED_TOOLING)
            find_program(DOTNET_EXECUTABLE dotnet REQUIRED)
            set(RML_MANAGED_SOLUTION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/RobloxModLoader.Managed.sln")

            add_custom_target(rml_managed_generate_solution
                    COMMAND "${DOTNET_EXECUTABLE}" new sln -n RobloxModLoader.Managed -f sln -o "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMAND "${DOTNET_EXECUTABLE}" sln "${RML_MANAGED_SOLUTION_PATH}" add ${RML_MANAGED_PROJECTS}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Generating managed solution and adding managed/tooling projects"
                    VERBATIM
            )
            set_target_properties(rml_managed_generate_solution PROPERTIES FOLDER "Managed")
        endif ()
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