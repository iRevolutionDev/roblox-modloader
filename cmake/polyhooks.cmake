include("${ROBLOX_MODLOADER_ROOT_DIR}/cmake/cpm.cmake")

CPMAddPackage(
        NAME polyhook2
        GITHUB_REPOSITORY stevemk14ebr/PolyHook_2_0
        GIT_TAG 49a95d4566d47342b122303cf73585cf22653b0a
        OPTIONS
        "POLYHOOK_BUILD_STATIC_RUNTIME OFF"
)
