if (COMMAND roblox_add_mod)
    return()
endif ()

function(roblox_add_mod MOD_NAME)
    cmake_parse_arguments(MOD "" "" "SOURCES;DEPENDENCIES;INCLUDE_DIRS;COMPILE_DEFINITIONS" ${ARGN})

    if (NOT TARGET ${MOD_NAME})
        if (NOT MOD_SOURCES)
            file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.h")
        endif ()
        add_library(${MOD_NAME} MODULE ${MOD_SOURCES})
    elseif (MOD_SOURCES)
        target_sources(${MOD_NAME} PRIVATE ${MOD_SOURCES})
    endif ()

    target_link_libraries(${MOD_NAME} PRIVATE RobloxModLoader::RobloxModLoader)

    if (MOD_DEPENDENCIES)
        target_link_libraries(${MOD_NAME} PRIVATE ${MOD_DEPENDENCIES})
    endif ()

    if (MOD_INCLUDE_DIRS)
        target_include_directories(${MOD_NAME} PRIVATE ${MOD_INCLUDE_DIRS})
    endif ()

    if (MOD_COMPILE_DEFINITIONS)
        target_compile_definitions(${MOD_NAME} PRIVATE ${MOD_COMPILE_DEFINITIONS})
    endif ()

    set_target_properties(${MOD_NAME} PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    if (MSVC)
        target_compile_options(${MOD_NAME} PRIVATE
                /bigobj
                /utf-8
                $<$<CONFIG:Debug>:/ZI>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Oi /Ot /Oy /Ob3 /sdl- /GL /GF /GS- /Gw>
        )
        target_link_options(${MOD_NAME} PRIVATE
                $<$<CONFIG:Debug>:/INCREMENTAL>
                $<$<CONFIG:RelWithDebInfo>:/LTCG /OPT:REF,ICF /GUARD:NO>
        )
    endif ()

    message(STATUS "Created RobloxModLoader mod: ${MOD_NAME}")
endfunction()
