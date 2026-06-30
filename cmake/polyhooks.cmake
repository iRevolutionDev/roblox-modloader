include(FetchContent)

set(POLYHOOK_BUILD_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic runtime for PolyHook_2" FORCE)

FetchContent_Declare(
        polyhook2
        GIT_REPOSITORY https://github.com/stevemk14ebr/PolyHook_2_0.git
        GIT_TAG 49a95d4566d47342b122303cf73585cf22653b0a
        GIT_SHALLOW TRUE
        UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(polyhook2)