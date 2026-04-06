include(cmake/CPM.cmake)

CPMAddPackage(
        NAME minhook
        GITHUB_REPOSITORY TsudaKageyu/minhook
        GIT_TAG master
        DOWNLOAD_ONLY YES
)

if (minhook_ADDED)
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

    if (MSVC)
        target_compile_options(minhook PRIVATE /W3)
        set_target_properties(minhook PROPERTIES
                COMPILE_WARNING_AS_ERROR OFF
        )
    endif ()

    set_target_properties(minhook PROPERTIES FOLDER "External/MinHook")
endif ()
