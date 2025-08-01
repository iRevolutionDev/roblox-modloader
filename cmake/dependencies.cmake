include(cmake/CPM.cmake)

function(setup_compiler_flags target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE
                /bigobj
                /utf-8
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
            "${luau_SOURCE_DIR}/Compiler/include"
            "${luau_SOURCE_DIR}/Ast/include"
            "${luau_SOURCE_DIR}/VM/include"
            "${luau_SOURCE_DIR}/VM/src"
            "${luau_SOURCE_DIR}/EqSat/include"
            "${luau_SOURCE_DIR}/CodeGen/include"
            "${luau_SOURCE_DIR}/Common/include"
    )
endfunction()

function(setup_core_dependencies target_name access_level)
    target_link_libraries(${target_name} ${access_level}
            spdlog::spdlog
            minhook
            ZLIB::ZLIB
            Tracy::TracyClient
            nlohmann_json::nlohmann_json
    )

    target_include_directories(${target_name} ${access_level}
            "${spdlog_SOURCE_DIR}"
            "${minhook_SOURCE_DIR}/include"
            "${zlib_SOURCE_DIR}"
            "${tomlplusplus_SOURCE_DIR}/include"
            "${tracy_SOURCE_DIR}/public"
            "${nlohmann_json_SOURCE_DIR}/include"
    )
endfunction()
