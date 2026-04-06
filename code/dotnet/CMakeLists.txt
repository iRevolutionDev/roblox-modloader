find_program(DOTNET_EXECUTABLE dotnet REQUIRED)

if (NOT DEFINED RML_DOTNET_CONFIGURATION)
    if (CMAKE_CONFIGURATION_TYPES)
        set(RML_DOTNET_CONFIGURATION "$<CONFIG>")
    elseif (CMAKE_BUILD_TYPE)
        set(RML_DOTNET_CONFIGURATION "${CMAKE_BUILD_TYPE}")
    else ()
        set(RML_DOTNET_CONFIGURATION "Release")
    endif ()
endif ()

set(RML_DOTNET_PUBLISH_DIR "${CMAKE_BINARY_DIR}/dotnet_out" CACHE PATH "Output dir for dotnet publish/pack artifacts")

file(GLOB_RECURSE RML_DOTNET_PROJECTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "*.csproj")
set(_filtered_projects)
foreach (_p IN LISTS RML_DOTNET_PROJECTS)
    if (_p MATCHES "(/|\\\\)(bin|obj)(/|\\\\)")
        # skip
    else ()
        list(APPEND _filtered_projects "${_p}")
    endif ()
endforeach ()
set(RML_DOTNET_PROJECTS ${_filtered_projects})

if (RML_DOTNET_PROJECTS)
    set(_restores)
    set(_builds)
    set(_publishes)
    set(_packs)

    foreach (_proj IN LISTS RML_DOTNET_PROJECTS)
        get_filename_component(_name ${_proj} NAME_WE)
        string(REGEX REPLACE "[^A-Za-z0-9]" "_" _safe_name ${_name})

        set(_restore_target rml_dotnet_restore_${_safe_name})
        set(_build_target rml_dotnet_build_${_safe_name})
        set(_publish_target rml_dotnet_publish_${_safe_name})
        set(_pack_target rml_dotnet_pack_${_safe_name})

        if (NOT TARGET ${_restore_target})
            add_custom_target(${_restore_target}
                    COMMAND "${DOTNET_EXECUTABLE}" restore "${CMAKE_CURRENT_SOURCE_DIR}/${_proj}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Restoring ${_proj}"
                    VERBATIM
            )
            set_target_properties(${_restore_target} PROPERTIES FOLDER "Dotnet")
        endif ()

        if (NOT TARGET ${_build_target})
            add_custom_target(${_build_target}
                    COMMAND "${DOTNET_EXECUTABLE}" build "${CMAKE_CURRENT_SOURCE_DIR}/${_proj}" -c "${RML_DOTNET_CONFIGURATION}" --no-restore
                    DEPENDS ${_restore_target}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Building ${_proj}"
                    VERBATIM
            )
            set_target_properties(${_build_target} PROPERTIES FOLDER "Dotnet")
        endif ()

        if (NOT TARGET ${_publish_target})
            add_custom_target(${_publish_target}
                    COMMAND "${DOTNET_EXECUTABLE}" publish "${CMAKE_CURRENT_SOURCE_DIR}/${_proj}" -c "${RML_DOTNET_CONFIGURATION}" -o "${RML_DOTNET_PUBLISH_DIR}/${_safe_name}" --no-restore
                    DEPENDS ${_build_target}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Publishing ${_proj} to ${RML_DOTNET_PUBLISH_DIR}/${_safe_name}"
                    VERBATIM
            )
            set_target_properties(${_publish_target} PROPERTIES FOLDER "Dotnet")
        endif ()

        if (NOT TARGET ${_pack_target})
            add_custom_target(${_pack_target}
                    COMMAND "${DOTNET_EXECUTABLE}" pack "${CMAKE_CURRENT_SOURCE_DIR}/${_proj}" -c "${RML_DOTNET_CONFIGURATION}" -o "${RML_DOTNET_PUBLISH_DIR}/nuget" --no-restore
                    DEPENDS ${_build_target}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Packing ${_proj} to ${RML_DOTNET_PUBLISH_DIR}/nuget"
                    VERBATIM
            )
            set_target_properties(${_pack_target} PROPERTIES FOLDER "Dotnet")
        endif ()

        list(APPEND _restores ${_restore_target})
        list(APPEND _builds ${_build_target})
        list(APPEND _publishes ${_publish_target})
        list(APPEND _packs ${_pack_target})
    endforeach ()

    if (NOT TARGET rml_dotnet_restore_all)
        add_custom_target(rml_dotnet_restore_all)
        add_dependencies(rml_dotnet_restore_all ${_restores})
        set_target_properties(rml_dotnet_restore_all PROPERTIES FOLDER "Dotnet")
    endif ()

    if (NOT TARGET rml_dotnet_build_all)
        add_custom_target(rml_dotnet_build_all)
        add_dependencies(rml_dotnet_build_all ${_builds})
        set_target_properties(rml_dotnet_build_all PROPERTIES FOLDER "Dotnet")
    endif ()

    if (NOT TARGET rml_dotnet_publish_all)
        add_custom_target(rml_dotnet_publish_all)
        add_dependencies(rml_dotnet_publish_all ${_publishes})
        set_target_properties(rml_dotnet_publish_all PROPERTIES FOLDER "Dotnet")
    endif ()

    if (NOT TARGET rml_dotnet_pack_all)
        add_custom_target(rml_dotnet_pack_all)
        add_dependencies(rml_dotnet_pack_all ${_packs})
        set_target_properties(rml_dotnet_pack_all PROPERTIES FOLDER "Dotnet")
    endif ()

    if (WIN32)
        if (NOT TARGET rml_dotnet_push_all)
            add_custom_target(rml_dotnet_push_all
                    COMMAND powershell -NoProfile -Command "Get-ChildItem -Path '${RML_DOTNET_PUBLISH_DIR}/nuget' -Filter '*.nupkg' -Recurse | ForEach-Object { & ${DOTNET_EXECUTABLE} nuget push $_.FullName --api-key $env:NUGET_API_KEY --source $env:NUGET_SOURCE --skip-duplicate }"
                    DEPENDS rml_dotnet_pack_all
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Pushing NuGet packages (requires NUGET_API_KEY/NUGET_SOURCE env vars)"
            )
        endif ()
    else ()
        if (NOT TARGET rml_dotnet_push_all)
            add_custom_target(rml_dotnet_push_all
                    COMMAND bash -lc "for f in '${RML_DOTNET_PUBLISH_DIR}/nuget'/*.nupkg; do [ -f \"\$f\" ] || continue; ${DOTNET_EXECUTABLE} nuget push \"\$f\" --api-key \"\$NUGET_API_KEY\" --source \"\$NUGET_SOURCE\" --skip-duplicate; done"
                    DEPENDS rml_dotnet_pack_all
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Pushing NuGet packages (requires NUGET_API_KEY/NUGET_SOURCE env vars)"
            )
        endif ()
    endif ()
endif ()
