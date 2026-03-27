include_guard(GLOBAL)

option(RML_ENABLE_DOTNET_HOSTING "Enable .NET hosting support" ON)
option(RML_DOTNET_LINK_NETHOST "Link nethost at build time (can cause early DLL dependency at process load)" OFF)

set(RML_DOTNET_CHANNEL "10.0" CACHE STRING
        ".NET release channel to pull from (e.g. 8.0, 9.0, 10.0)")

function(_rml_detect_rid out_rid out_ext out_libname)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)

    if (_proc MATCHES "^(x86_64|amd64)$")
        set(_arch "x64")
    elseif (_proc MATCHES "^(aarch64|arm64)$")
        set(_arch "arm64")
    elseif (_proc MATCHES "^arm")
        set(_arch "arm")
    else ()
        message(FATAL_ERROR "[DotNetHosting] Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif ()

    if (WIN32)
        set(_os "win")
        set(_ext "zip")
        set(_lib "nethost.lib")
    elseif (APPLE)
        set(_os "osx")
        set(_ext "tar.gz")
        set(_lib "libnethost.a")
    elseif (UNIX)
        set(_os "linux")
        set(_ext "tar.gz")
        set(_lib "libnethost.a")
    else ()
        message(FATAL_ERROR "[DotNetHosting] Unsupported OS: ${CMAKE_SYSTEM_NAME}")
    endif ()

    set(${out_rid} "${_os}-${_arch}" PARENT_SCOPE)
    set(${out_ext} "${_ext}" PARENT_SCOPE)
    set(${out_libname} "${_lib}" PARENT_SCOPE)
endfunction()

function(_rml_detect_nethost_runtime_name out_name)
    if (WIN32)
        set(${out_name} "nethost.dll" PARENT_SCOPE)
    elseif (APPLE)
        set(${out_name} "libnethost.dylib" PARENT_SCOPE)
    else ()
        set(${out_name} "libnethost.so" PARENT_SCOPE)
    endif ()
endfunction()

function(_rml_json_get_string json key out_value)
    string(REGEX MATCH "\"${key}\":\"([^\"]+)\"" _m "${json}")
    if (CMAKE_MATCH_1)
        set(${out_value} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else ()
        set(${out_value} "" PARENT_SCOPE)
    endif ()
endfunction()

function(_rml_download url dest label)
    if (EXISTS "${dest}")
        message(STATUS "[DotNetHosting] Using cached ${label}")
        return()
    endif ()

    message(STATUS "[DotNetHosting] Downloading ${label} ...")
    file(DOWNLOAD "${url}" "${dest}"
            STATUS _status
            TLS_VERIFY ON
            SHOW_PROGRESS
    )
    list(GET _status 0 _code)
    list(GET _status 1 _msg)
    if (NOT _code EQUAL 0)
        file(REMOVE "${dest}")
        message(FATAL_ERROR "[DotNetHosting] Download failed (${_code}): ${_msg}\n  URL: ${url}")
    endif ()
endfunction()

function(_rml_acquire_dotnet_hosting out_include_dir out_lib_path out_runtime_root)
    _rml_detect_rid(_rid _ext _libname)
    _rml_detect_nethost_runtime_name(_runtime_name)
    message(STATUS "[DotNetHosting] Target RID: ${_rid}")

    set(_cache_dir "${CMAKE_BINARY_DIR}/_dotnet_hosting_cache")
    file(MAKE_DIRECTORY "${_cache_dir}")

    set(_meta_url "https://dotnetcli.blob.core.windows.net/dotnet/release-metadata/${RML_DOTNET_CHANNEL}/releases.json")
    set(_meta_path "${_cache_dir}/releases-${RML_DOTNET_CHANNEL}.json")
    _rml_download("${_meta_url}" "${_meta_path}" "release metadata for .NET ${RML_DOTNET_CHANNEL}")

    file(READ "${_meta_path}" _raw)

    string(REGEX REPLACE "[ \t\r\n]+" "" _json "${_raw}")

    _rml_json_get_string("${_json}" "latest-runtime" _runtime_ver)
    if (NOT _runtime_ver)
        message(FATAL_ERROR "[DotNetHosting] Could not parse 'latest-runtime' from releases.json")
    endif ()
    set(RML_DOTNET_RUNTIME_VERSION "${_runtime_ver}" CACHE INTERNAL "")
    message(STATUS "[DotNetHosting] Latest runtime: ${_runtime_ver}")

    set(_pack_pattern "https://[^\"]*dotnet-apphost-pack-${_runtime_ver}-${_rid}\\.${_ext}")
    string(REGEX MATCH "${_pack_pattern}" _pack_url "${_json}")

    if (NOT _pack_url)
        set(_base "https://builds.dotnet.microsoft.com/dotnet/Runtime/${_runtime_ver}")
        set(_pack_url "${_base}/dotnet-apphost-pack-${_runtime_ver}-${_rid}.${_ext}")
        message(STATUS "[DotNetHosting] Apphost pack URL not found in metadata, using direct URL")
    endif ()

    set(_runtime_pattern "https://[^\"]*dotnet-runtime-${_runtime_ver}-${_rid}\\.${_ext}")
    string(REGEX MATCH "${_runtime_pattern}" _runtime_url "${_json}")

    if (NOT _runtime_url)
        set(_base "https://builds.dotnet.microsoft.com/dotnet/Runtime/${_runtime_ver}")
        set(_runtime_url "${_base}/dotnet-runtime-${_runtime_ver}-${_rid}.${_ext}")
        message(STATUS "[DotNetHosting] Runtime URL not found in metadata, using direct URL")
    endif ()

    message(STATUS "[DotNetHosting] Apphost URL: ${_pack_url}")
    message(STATUS "[DotNetHosting] Runtime URL: ${_runtime_url}")

    set(_pack_filename "dotnet-apphost-pack-${_runtime_ver}-${_rid}.${_ext}")
    set(_pack_archive "${_cache_dir}/${_pack_filename}")
    _rml_download("${_pack_url}" "${_pack_archive}" "apphost pack ${_runtime_ver} (${_rid})")

    set(_runtime_filename "dotnet-runtime-${_runtime_ver}-${_rid}.${_ext}")
    set(_runtime_archive "${_cache_dir}/${_runtime_filename}")
    _rml_download("${_runtime_url}" "${_runtime_archive}" "runtime ${_runtime_ver} (${_rid})")

    set(_pack_extract_dir "${_cache_dir}/apphost-pack-${_runtime_ver}-${_rid}")
    set(_pack_extract_stamp "${_pack_extract_dir}/.extracted")
    if (NOT EXISTS "${_pack_extract_stamp}")
        message(STATUS "[DotNetHosting] Extracting ${_pack_filename} ...")
        file(MAKE_DIRECTORY "${_pack_extract_dir}")
        file(ARCHIVE_EXTRACT INPUT "${_pack_archive}" DESTINATION "${_pack_extract_dir}")
        file(WRITE "${_pack_extract_stamp}" "${_runtime_ver}")
    else ()
        message(STATUS "[DotNetHosting] Using cached extraction for ${_pack_filename}")
    endif ()

    set(_runtime_extract_dir "${_cache_dir}/runtime-${_runtime_ver}-${_rid}")
    set(_runtime_extract_stamp "${_runtime_extract_dir}/.extracted")
    if (NOT EXISTS "${_runtime_extract_stamp}")
        message(STATUS "[DotNetHosting] Extracting ${_runtime_filename} ...")
        file(MAKE_DIRECTORY "${_runtime_extract_dir}")
        file(ARCHIVE_EXTRACT INPUT "${_runtime_archive}" DESTINATION "${_runtime_extract_dir}")
        file(WRITE "${_runtime_extract_stamp}" "${_runtime_ver}")
    else ()
        message(STATUS "[DotNetHosting] Using cached extraction for ${_runtime_filename}")
    endif ()

    file(GLOB_RECURSE _header_candidates
                "${_pack_extract_dir}/nethost.h"
                "${_pack_extract_dir}/*/nethost.h"
                "${_pack_extract_dir}/*/*/nethost.h"
                "${_pack_extract_dir}/*/*/*/nethost.h"
                "${_pack_extract_dir}/*/*/*/*/nethost.h"
    )

    if (NOT _header_candidates)
        message(FATAL_ERROR
                "[DotNetHosting] nethost.h not found after extraction.\n"
                "  Extraction dir: ${_pack_extract_dir}\n"
                "  Please report this at https://github.com/iRevolutionDev/roblox-modloader/issues")
    endif ()

    list(GET _header_candidates 0 _nethost_h)
    get_filename_component(_include_dir "${_nethost_h}" DIRECTORY)

    foreach (_hdr hostfxr.h coreclr_delegates.h)
        if (NOT EXISTS "${_include_dir}/${_hdr}")
            message(WARNING "[DotNetHosting] Expected header not found: ${_include_dir}/${_hdr}")
        endif ()
    endforeach ()

    file(GLOB_RECURSE _lib_candidates
            "${_pack_extract_dir}/${_libname}"
            "${_pack_extract_dir}/*/${_libname}"
            "${_pack_extract_dir}/*/*/${_libname}"
            "${_pack_extract_dir}/*/*/*/${_libname}"
            "${_pack_extract_dir}/*/*/*/*/${_libname}"
    )

    if (_lib_candidates)
        list(GET _lib_candidates 0 _lib_path)
    else ()
        set(_lib_path "")
        message(WARNING "[DotNetHosting] ${_libname} not found (link-time nethost import may be unavailable)")
    endif ()

    file(GLOB_RECURSE _runtime_lib_candidates
            "${_runtime_extract_dir}/${_runtime_name}"
            "${_runtime_extract_dir}/*/${_runtime_name}"
            "${_runtime_extract_dir}/*/*/${_runtime_name}"
            "${_runtime_extract_dir}/*/*/*/${_runtime_name}"
            "${_runtime_extract_dir}/*/*/*/*/${_runtime_name}"
    )

    if (_runtime_lib_candidates)
        list(GET _runtime_lib_candidates 0 _runtime_lib_path)
        set(RML_DOTNET_NETHOST_RUNTIME_BINARY "${_runtime_lib_path}" CACHE INTERNAL "")
    else ()
        set(RML_DOTNET_NETHOST_RUNTIME_BINARY "" CACHE INTERNAL "")
        message(WARNING "[DotNetHosting] Runtime nethost binary (${_runtime_name}) not found in extracted runtime")
    endif ()

    if (NOT EXISTS "${_runtime_extract_dir}/host/fxr")
        message(FATAL_ERROR "[DotNetHosting] host/fxr not found in extracted runtime: ${_runtime_extract_dir}")
    endif ()

    if (NOT EXISTS "${_runtime_extract_dir}/shared/Microsoft.NETCore.App")
        message(FATAL_ERROR "[DotNetHosting] shared/Microsoft.NETCore.App not found in extracted runtime: ${_runtime_extract_dir}")
    endif ()

    set(${out_include_dir} "${_include_dir}" PARENT_SCOPE)
    set(${out_lib_path} "${_lib_path}" PARENT_SCOPE)
    set(${out_runtime_root} "${_runtime_extract_dir}" PARENT_SCOPE)
endfunction()

if (RML_ENABLE_DOTNET_HOSTING)
    _rml_acquire_dotnet_hosting(
            _rml_dotnet_include_dir
            _rml_dotnet_lib_path
            _rml_dotnet_runtime_root
    )

    message(STATUS "[DotNetHosting] Include dir : ${_rml_dotnet_include_dir}")
    message(STATUS "[DotNetHosting] Library     : ${_rml_dotnet_lib_path}")
    message(STATUS "[DotNetHosting] Runtime root: ${_rml_dotnet_runtime_root}")

    if (NOT TARGET DotNetHosting::headers)
        add_library(_dotnet_hosting_headers INTERFACE)
        target_include_directories(_dotnet_hosting_headers INTERFACE
                "${_rml_dotnet_include_dir}")
        add_library(DotNetHosting::headers ALIAS _dotnet_hosting_headers)
    endif ()

    if (NOT TARGET DotNetHosting::nethost AND _rml_dotnet_lib_path AND EXISTS "${_rml_dotnet_lib_path}")
        add_library(_dotnet_nethost UNKNOWN IMPORTED GLOBAL)
        set_target_properties(_dotnet_nethost PROPERTIES
                IMPORTED_LOCATION "${_rml_dotnet_lib_path}")
        add_library(DotNetHosting::nethost ALIAS _dotnet_nethost)
    endif ()

    if (NOT TARGET DotNetHosting::hosting)
        add_library(_dotnet_hosting INTERFACE)
        target_link_libraries(_dotnet_hosting INTERFACE
                DotNetHosting::headers)

        if (RML_DOTNET_LINK_NETHOST)
            if (NOT TARGET DotNetHosting::nethost)
                message(FATAL_ERROR "[DotNetHosting] RML_DOTNET_LINK_NETHOST=ON but nethost import library is unavailable for this runtime payload.")
            endif ()
            target_link_libraries(_dotnet_hosting INTERFACE DotNetHosting::nethost)
        endif ()

        if (UNIX)
            target_link_libraries(_dotnet_hosting INTERFACE dl)
        endif ()
        add_library(DotNetHosting::hosting ALIAS _dotnet_hosting)
    endif ()

    set(RML_DOTNET_HOSTING_INCLUDE_DIR "${_rml_dotnet_include_dir}" CACHE INTERNAL "")
    set(RML_DOTNET_NETHOST_LIBRARY "${_rml_dotnet_lib_path}" CACHE INTERNAL "")
    set(RML_DOTNET_RUNTIME_ROOT "${_rml_dotnet_runtime_root}" CACHE INTERNAL "")

    if (RML_DOTNET_RUNTIME_VERSION)
        set(_rml_tfm "net${RML_DOTNET_CHANNEL}")
        set(RML_DOTNET_GLOBAL_RUNTIME_CONFIG "${CMAKE_BINARY_DIR}/RobloxModLoader.runtimeconfig.json" CACHE INTERNAL "")
        file(WRITE "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "{\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "  \"runtimeOptions\": {\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "    \"tfm\": \"${_rml_tfm}\",\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "    \"rollForward\": \"Minor\",\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "    \"framework\": {\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "      \"name\": \"Microsoft.NETCore.App\",\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "      \"version\": \"${RML_DOTNET_RUNTIME_VERSION}\"\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "    }\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "  }\n")
        file(APPEND "${RML_DOTNET_GLOBAL_RUNTIME_CONFIG}" "}\n")
    endif ()
endif ()

function(setup_dotnet_hosting target access)
    if (NOT RML_ENABLE_DOTNET_HOSTING)
        return()
    endif ()
    if (NOT TARGET DotNetHosting::hosting)
        message(FATAL_ERROR "[DotNetHosting] Target unavailable. Include dotnet_hosting.cmake first.")
    endif ()

    target_link_libraries(${target} ${access} DotNetHosting::hosting)
    if (RML_DOTNET_LINK_NETHOST)
        if (NOT TARGET DotNetHosting::nethost)
            message(FATAL_ERROR "[DotNetHosting] DotNetHosting::nethost is unavailable while RML_DOTNET_LINK_NETHOST=ON")
        endif ()
        target_link_libraries(${target} ${access} DotNetHosting::nethost)
    endif ()
    target_compile_definitions(${target} ${access}
            RML_DOTNET_HOSTING=1
    )
    if (RML_DOTNET_RUNTIME_ROOT)
        file(TO_CMAKE_PATH "${RML_DOTNET_RUNTIME_ROOT}" _rt_path)
        target_compile_definitions(${target} ${access}
                RML_DOTNET_RUNTIME_ROOT="${_rt_path}"
        )
    endif ()
endfunction()