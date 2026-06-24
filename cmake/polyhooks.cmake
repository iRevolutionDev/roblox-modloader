include(FetchContent)

set(POLYHOOK_BUILD_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic runtime for PolyHook_2" FORCE)

FetchContent_Declare(
        polyhook2
        GIT_REPOSITORY https://github.com/stevemk14ebr/PolyHook_2_0.git
        GIT_TAG f4aee8e47383825469f924903357038b2efd8ca7
        GIT_SHALLOW TRUE
        UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(polyhook2)