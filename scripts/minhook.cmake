include(FetchContent)

message("MinHook")
FetchContent_Declare(
        minhook
        GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(minhook)
if(NOT minhook_POPULATED)
    FetchContent_Populate(minhook)
    
    set(MINHOOK_SRC
        "${minhook_SOURCE_DIR}/src/buffer.c"
        "${minhook_SOURCE_DIR}/src/hook.c"
        "${minhook_SOURCE_DIR}/src/trampoline.c"
        "${minhook_SOURCE_DIR}/src/hde/hde32.c"
        "${minhook_SOURCE_DIR}/src/hde/hde64.c"
    )
    
    add_library(minhook STATIC ${MINHOOK_SRC})
    target_include_directories(minhook PUBLIC 
        "${minhook_SOURCE_DIR}/include"
        "${minhook_SOURCE_DIR}/src"
        "${minhook_SOURCE_DIR}/src/hde"
    )

    if(MSVC)
        target_compile_options(minhook PRIVATE /W3)
    endif()
endif()
