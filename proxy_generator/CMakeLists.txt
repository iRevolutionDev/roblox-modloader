cmake_minimum_required(VERSION 3.25)

enable_language(RC)

add_executable(proxy_generator
        main.cpp
        proxy.rc
)

target_compile_features(proxy_generator PRIVATE cxx_std_23)

target_link_libraries(proxy_generator PRIVATE
        imagehlp
        user32
)

target_compile_definitions(proxy_generator PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
)

set_target_properties(proxy_generator PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "proxy_generator"
        FOLDER "Tools"
)

option(RML_PROXY_DLL_PATH "Path to the DLL to create proxy for" "C:/Windows/System32/dwmapi.dll")

function(generate_proxy_dll TARGET_DLL_PATH OUTPUT_DIR)
    get_filename_component(DLL_NAME ${TARGET_DLL_PATH} NAME_WE)

    add_custom_target(generate_proxy_${DLL_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
            COMMAND $<TARGET_FILE:proxy_generator> ${TARGET_DLL_PATH} ${OUTPUT_DIR}
            DEPENDS proxy_generator
            COMMENT "Generating proxy for ${TARGET_DLL_PATH}"
            VERBATIM
    )

    add_library(${DLL_NAME}_proxy SHARED
            ${OUTPUT_DIR}/dllmain.cpp
            ${OUTPUT_DIR}/${DLL_NAME}.def
            ${OUTPUT_DIR}/${DLL_NAME}.asm
            ${OUTPUT_DIR}/proxy.rc
    )

    enable_language(RC)

    set_target_properties(${DLL_NAME}_proxy PROPERTIES
            OUTPUT_NAME ${DLL_NAME}
            SUFFIX ".dll"
            LINK_FLAGS "/DEF:${OUTPUT_DIR}/${DLL_NAME}.def"
    )

    enable_language(ASM_MASM)

    add_dependencies(${DLL_NAME}_proxy generate_proxy_${DLL_NAME})

    target_link_libraries(${DLL_NAME}_proxy PRIVATE
            user32
            kernel32
    )

    target_compile_definitions(${DLL_NAME}_proxy PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
endfunction()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND DEFINED RML_PROXY_DLL_PATH AND RML_PROXY_DLL_PATH)
    set(PROXY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated_proxy")
    generate_proxy_dll(${RML_PROXY_DLL_PATH} ${PROXY_OUTPUT_DIR})
endif ()

install(TARGETS proxy_generator
        RUNTIME DESTINATION bin
        COMPONENT Tools
)
