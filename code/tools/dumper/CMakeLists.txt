file(GLOB_RECURSE DUMPER_CORE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER DUMPER_CORE_SOURCES EXCLUDE REGEX "/src/cli/main\\.cpp$")

add_library(dumper_core STATIC ${DUMPER_CORE_SOURCES})

target_compile_features(dumper_core PUBLIC cxx_std_23)

target_include_directories(dumper_core PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(dumper_core PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(dumper_core PUBLIC
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)
target_link_libraries(dumper_core PRIVATE
        Zydis
        capstone
        ZLIB::ZLIB
)

if (RML_PLATFORM_WINDOWS)
    target_link_libraries(dumper_core PRIVATE winhttp)
else ()
    find_package(CURL REQUIRED)
    target_link_libraries(dumper_core PRIVATE CURL::libcurl)
endif ()

set_target_properties(dumper_core PROPERTIES FOLDER "Tools")

add_executable(dumper "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.cpp")

target_include_directories(dumper PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(dumper PRIVATE dumper_core)

set_target_properties(dumper PROPERTIES
        OUTPUT_NAME "dumper"
        FOLDER "Tools"
)

if (ROBLOX_MODLOADER_BUILD_DUMPER_TESTS)
    file(GLOB_RECURSE DUMPER_TEST_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    add_executable(dumper_tests ${DUMPER_TEST_SOURCES})

    target_include_directories(dumper_tests PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    target_link_libraries(dumper_tests PRIVATE dumper_core doctest::doctest ZLIB::ZLIB)

    set_target_properties(dumper_tests PROPERTIES FOLDER "Tools")

    add_test(NAME dumper_tests COMMAND dumper_tests)
endif ()
