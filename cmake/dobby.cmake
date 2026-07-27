include(cmake/CPM.cmake)

CPMAddPackage(
        NAME dobby
        GITHUB_REPOSITORY jmpews/Dobby
        GIT_TAG master
        OPTIONS
        "DOBBY_DEBUG OFF"
        "DOBBY_BUILD_EXAMPLE OFF"
        "DOBBY_BUILD_TEST OFF"
        "Plugin.SymbolResolver ON"
        "Plugin.ImportTableReplace OFF"
)
