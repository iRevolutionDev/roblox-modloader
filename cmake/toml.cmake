include(FetchContent)

message("TOML")
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus
        GIT_TAG v3.4.0
        GIT_PROGRESS TRUE
)

add_compile_definitions(TOML_EXCEPTIONS=0)
FetchContent_MakeAvailable(tomlplusplus)