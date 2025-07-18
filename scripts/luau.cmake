include(FetchContent)

message("Luau")

set(LUAU_BUILD_CLI OFF CACHE BOOL "Build Luau CLI" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "Build Luau tests" FORCE)
set(LUAU_BUILD_WEB OFF CACHE BOOL "Build Luau web components" FORCE)

FetchContent_Declare(
        luau
        GIT_REPOSITORY https://github.com/luau-lang/luau
        GIT_TAG 0.680
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(luau)