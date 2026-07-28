include(cmake/cpm.cmake)

if (NOT TARGET doctest::doctest)
    set(RML_POLICY_MINIMUM_BEFORE_DOCTEST "${CMAKE_POLICY_VERSION_MINIMUM}")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

    CPMAddPackage(
            NAME doctest
            VERSION 2.4.11
            GITHUB_REPOSITORY doctest/doctest
            OPTIONS
            "DOCTEST_WITH_TESTS OFF"
            "DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF"
    )

    set(CMAKE_POLICY_VERSION_MINIMUM "${RML_POLICY_MINIMUM_BEFORE_DOCTEST}")
endif ()
