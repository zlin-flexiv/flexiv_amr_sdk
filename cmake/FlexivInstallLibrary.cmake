macro(FlexivInstallLibrary)
    include(GNUInstallDirs)

    install(
        TARGETS ${FLEXIV_AMR_TARGET_NAME}
        EXPORT flexiv_amr_sdkTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
    )

    install(
        EXPORT flexiv_amr_sdkTargets
        FILE flexiv_amr_sdkTargets.cmake
        NAMESPACE flexiv::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/flexiv_amr_sdk"
    )

    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/flexiv_amr_sdk-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/flexiv_amr_sdkConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/flexiv_amr_sdk"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/flexiv_amr_sdkConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/flexiv_amr_sdkConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/flexiv_amr_sdkConfigVersion.cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findasio.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/flexiv_amr_sdk"
    )

    install(
        CODE
        "file(REMOVE \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${FLEXIV_AMR_TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}\")"
    )

    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/${FLEXIV_AMR_LIB}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RENAME "${CMAKE_STATIC_LIBRARY_PREFIX}${FLEXIV_AMR_TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/example/"
        DESTINATION example
        FILES_MATCHING
            PATTERN "*.cpp"
            PATTERN "CMakeLists.txt"
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/example_py/"
        DESTINATION example_py
        FILES_MATCHING
            PATTERN "*.py"
    )

    install(
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION .
    )

    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/VERSION\" \"v${PROJECT_VERSION}\\n\")"
    )
endmacro()
