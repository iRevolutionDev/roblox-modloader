if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR)
        if (WIN32)
            add_subdirectory(code/tools/proxy_generator)
        else ()
            message(STATUS "Skipping proxy_generator: only supported on Windows")
        endif ()
    endif ()

    if (ROBLOX_MODLOADER_BUILD_DUMPER)
        add_subdirectory(code/tools/dumper)
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
endif ()
