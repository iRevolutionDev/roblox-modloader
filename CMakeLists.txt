cmake_minimum_required(VERSION 3.22.1)

if (WIN32)
    set(RML_DEFAULT_BUILD_PROXY_GENERATOR ON)
    set(RML_DEFAULT_BUILD_PROXY_DLL ON)
    set(RML_DEFAULT_BUILD_MANAGED_PROJECTS ON)
    set(RML_DEFAULT_USE_CMAKE_CSHARP ON)
else ()
    set(RML_DEFAULT_BUILD_PROXY_GENERATOR OFF)
    set(RML_DEFAULT_BUILD_PROXY_DLL OFF)
    set(RML_DEFAULT_BUILD_MANAGED_PROJECTS ON)
    set(RML_DEFAULT_USE_CMAKE_CSHARP OFF)
endif ()

project(RobloxModLoader LANGUAGES CXX C ASM VERSION 0.1.0 DESCRIPTION "Roblox ModLoader")

if (WIN32)
    set(RML_PLATFORM_WINDOWS ON)
    set(RML_PLATFORM_ID "windows")
elseif (APPLE)
    set(RML_PLATFORM_MACOS ON)
    set(RML_PLATFORM_ID "macos")
elseif (UNIX)
    set(RML_PLATFORM_LINUX ON)
    set(RML_PLATFORM_ID "linux")
else ()
    message(FATAL_ERROR "RobloxModLoader: unsupported target platform")
endif ()

if (WIN32)
    enable_language(ASM_MASM)
    enable_language(RC)
endif ()

option(ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR "Build the proxy generator tool" ${RML_DEFAULT_BUILD_PROXY_GENERATOR})
option(ROBLOX_MODLOADER_BUILD_PROXY_DLL "Build the dwmapi.dll proxy automatically" ${RML_DEFAULT_BUILD_PROXY_DLL})
option(ROBLOX_MODLOADER_BUILD_DUMPER "Build the Luau structures dumper tool" ON)
option(ROBLOX_MODLOADER_BUILD_EXAMPLES "Build example mods" ON)
option(ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS "Build C# projects as part of the default ALL target" ${RML_DEFAULT_BUILD_MANAGED_PROJECTS})
option(ROBLOX_MODLOADER_USE_CMAKE_CSHARP "Use CMake's native C# project integration on Visual Studio generators" ${RML_DEFAULT_USE_CMAKE_CSHARP})
option(ROBLOX_MODLOADER_INSTALL "Install RobloxModLoader" OFF)
option(RML_ENABLE_LUAU "Compile the disabled Luau scripting subsystem (layout-sensitive; VM structs pending re-confirmation via code/tools/dumper)" OFF)
option(RML_NATIVE_ONLY "Build the reduced native-mod runtime without Luau, .NET, Qt, scheduler, or engine hooks" OFF)

if (RML_NATIVE_ONLY)
    set(RML_ENABLE_LUAU OFF CACHE BOOL "Compile the Luau scripting subsystem" FORCE)
    set(ROBLOX_MODLOADER_BUILD_DUMPER OFF CACHE BOOL "Build the Luau structures dumper tool" FORCE)
    set(ROBLOX_MODLOADER_BUILD_DUMPER_TESTS OFF CACHE BOOL "Build the dumper unit tests" FORCE)
    set(ROBLOX_MODLOADER_BUILD_MANAGED_PROJECTS OFF CACHE BOOL "Build C# projects" FORCE)
    set(ROBLOX_MODLOADER_USE_CMAKE_CSHARP OFF CACHE BOOL "Use CMake's C# integration" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0 _HAS_ITERATOR_DEBUGGING=0)
endif ()

set(ROBLOX_MODLOADER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ROBLOX_MODLOADER_SOURCE_DIR "code/roblox_modloader/src")
set(ROBLOX_MODLOADER_INCLUDE_DIR "code/roblox_modloader/include")
set(LUAU_SOURCE_DIR "${ROBLOX_MODLOADER_ROOT_DIR}/libs/third_party/luau")

include(cmake/spdlog.cmake)
include(cmake/toml.cmake)

if (NOT RML_NATIVE_ONLY)
    include(cmake/zlib.cmake)
    #include(cmake/tracy.cmake)
    include(cmake/luau.cmake)
    include(cmake/json.cmake)

    if (RML_PLATFORM_WINDOWS)
        include(cmake/minhook.cmake)
        include(cmake/polyhooks.cmake)
    else ()
        include(cmake/dobby.cmake)
    endif ()
endif ()

if (NOT RML_NATIVE_ONLY)
    include(cmake/dotnet_hosting.cmake)
endif ()

include(cmake/dependencies.cmake)

include(cmake/roblox_add_mod.cmake)

if (NOT RML_NATIVE_ONLY)
    add_subdirectory(libs/third_party)
endif ()

add_subdirectory(code/roblox_modloader)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND NOT RML_NATIVE_ONLY)
    add_subdirectory(code/dotnet)
endif ()

include(cmake/subdirs.cmake)

include(cmake/install.cmake)
