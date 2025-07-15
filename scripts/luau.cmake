include(FetchContent)

message("Luau")
FetchContent_Declare(
        luau
        GIT_REPOSITORY https://github.com/luau-lang/luau
        GIT_TAG 0.680
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(luau)