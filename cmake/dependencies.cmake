include(cmake/CPM.cmake)

function(setup_compiler_flags target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE
                /bigobj
                /utf-8
                $<$<CONFIG:Debug>:/MDd>
                $<$<CONFIG:Release>:/MD>
                $<$<CONFIG:RelWithDebInfo>:/MD>
                $<$<CONFIG:MinSizeRel>:/MD>
                $<$<CONFIG:Debug>:/ZI>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Oi /Ot /Oy /Ob3 /sdl- /GL /GF /GS- /Gw>
        )
        target_link_options(${target_name} PRIVATE
                $<$<CONFIG:Debug>:/INCREMENTAL>
                $<$<CONFIG:RelWithDebInfo>:/LTCG /OPT:REF,ICF /GUARD:NO>
                /NXCOMPAT:NO
        )
    else ()
        target_compile_options(${target_name} PRIVATE
                -Wa,-mbig-obj
                -m32
        )
    endif ()
endfunction()

function(setup_compile_definitions target_name access_level)
    target_compile_definitions(${target_name} ${access_level}
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            IS_RML=1
            $<$<CONFIG:Debug>:DEBUG>
    )
endfunction()

function(setup_luau_dependencies target_name access_level)
    target_link_libraries(${target_name} ${access_level}
            Luau.Compiler
            Luau.Ast
            Luau.VM
            Luau.VM.Internals
            Luau.EqSat
            Luau.CodeGen
    )

    target_include_directories(${target_name} ${access_level}
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Compiler/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Ast/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/VM/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/VM/src"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/EqSat/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/CodeGen/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau/Common/include"
    )
endfunction()

function(setup_core_dependencies target_name access_level)
    target_link_libraries(${target_name} ${access_level}
            spdlog::spdlog
            minhook
            ZLIB::ZLIB
            #Tracy::TracyClient
            nlohmann_json::nlohmann_json
            PolyHook_2
    )

    target_include_directories(${target_name} ${access_level}
            "${spdlog_SOURCE_DIR}"
            "${minhook_SOURCE_DIR}/include"
            "${zlib_SOURCE_DIR}"
            "${tomlplusplus_SOURCE_DIR}/include"
            #"${tracy_SOURCE_DIR}/public"
            "${nlohmann_json_SOURCE_DIR}/include"
            "${polyhook2_SOURCE_DIR}"
    )
endfunction()
