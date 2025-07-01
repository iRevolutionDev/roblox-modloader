message(STATUS "Configuring examples")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB EXAMPLES_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*) # Get all directories in the current directory
foreach (EXAMPLE_DIR ${EXAMPLES_DIRS})
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR}/main.cpp)
        message(STATUS "Adding ${EXAMPLE_DIR}")
        add_library(${EXAMPLE_DIR} MODULE ${EXAMPLE_DIR}/main.cpp)
        file(GLOB_RECURSE EXAMPLES_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR}/*.hpp)
        target_sources(${EXAMPLE_DIR} PRIVATE ${EXAMPLES_FILES})

        target_include_directories(${EXAMPLE_DIR} PRIVATE
                "${PROJECT_SOURCE_DIR}/include"
                "${PROJECT_SOURCE_DIR}/src"
        )
        target_link_libraries(${EXAMPLE_DIR}
                ${PROJECT_NAME}_interface
                spdlog
                minhook
                ZLIB::ZLIB
        )
    else ()
        message(STATUS "Skipping ${EXAMPLE_DIR}")
    endif ()
endforeach ()
