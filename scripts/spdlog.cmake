include(cmake/CPM.cmake)

CPMAddPackage(
        NAME spdlog
        VERSION 1.15.0
        GITHUB_REPOSITORY gabime/spdlog
        OPTIONS
        "SPDLOG_BUILD_SHARED OFF"
        "SPDLOG_BUILD_EXAMPLE OFF"
        "SPDLOG_BUILD_TESTS OFF"
        "SPDLOG_INSTALL OFF"
)