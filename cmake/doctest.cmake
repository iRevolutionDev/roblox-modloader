include(cmake/cpm.cmake)

if (NOT TARGET doctest::doctest)
    CPMAddPackage(
            NAME doctest
            VERSION 2.4.11
            GITHUB_REPOSITORY doctest/doctest
            OPTIONS
            "DOCTEST_WITH_TESTS OFF"
            "DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF"
    )
endif ()
