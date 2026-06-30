if (ROBLOX_MODLOADER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY "${ROBLOX_MODLOADER_INCLUDE_DIR}/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    )

    install(TARGETS RobloxModLoader
            EXPORT RobloxModLoaderTargets
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RobloxModLoaderConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/RobloxModLoaderConfigVersion.cmake"
            "${ROBLOX_MODLOADER_ROOT_DIR}/cmake/roblox_add_mod.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )

    install(EXPORT RobloxModLoaderTargets
            FILE RobloxModLoaderTargets.cmake
            NAMESPACE RobloxModLoader::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RobloxModLoader
    )
endif ()
