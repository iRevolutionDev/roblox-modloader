message(STATUS "Configuring examples")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(create_mod_target MOD_NAME MOD_DIR)
    message(STATUS "Creating mod: ${MOD_NAME}")

    file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS
            "${MOD_DIR}/*.cpp"
            "${MOD_DIR}/*.hpp"
            "${MOD_DIR}/*.h"
    )

    add_library(${MOD_NAME} MODULE ${MOD_SOURCES})

    set_target_properties(${MOD_NAME} PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            COMPILE_WARNING_AS_ERROR ON
            FOLDER "Examples"
    )

    setup_compiler_flags(${MOD_NAME})

    target_link_libraries(${MOD_NAME} PRIVATE
            ${PROJECT_NAME}_interface
    )

    source_group(TREE ${MOD_DIR} PREFIX "src" FILES ${MOD_SOURCES})
endfunction()

file(GLOB EXAMPLE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach (EXAMPLE_DIR ${EXAMPLE_DIRS})
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR} AND
            EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR}/main.cpp)

        create_mod_target(${EXAMPLE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR})

    else ()
        message(STATUS "Skipping ${EXAMPLE_DIR} (not a valid mod directory)")
    endif ()
endforeach ()
