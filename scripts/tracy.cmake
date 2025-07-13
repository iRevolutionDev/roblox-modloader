include(FetchContent)

message("Tracy")
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.12.2
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(tracy)

if (WIN32)
    set(TRACY_ENABLE 1 CACHE BOOL "Enable Tracy profiler")
    set(TRACY_SYMBOL_OFFLINE_RESOLVE 1 CACHE BOOL "Enable offline symbol resolving for Tracy profiler")
    set(TRACY_CALLSTACK 10 CACHE BOOL "Enable callstack support for Tracy profiler")
    set(TRACY_FIBERS 1 CACHE BOOL "Enable fiber context support for Tracy profiler")
else ()
    set(TRACY_ENABLE 0 CACHE BOOL "Enable Tracy profiler")
endif ()