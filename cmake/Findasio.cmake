find_path(
    ASIO_INCLUDE_DIR
    NAMES asio.hpp
    PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio DEFAULT_MSG ASIO_INCLUDE_DIR)

if(asio_FOUND AND NOT TARGET asio::asio)
    add_library(asio::asio INTERFACE IMPORTED)

    set_target_properties(asio::asio PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS "ASIO_STANDALONE;ASIO_NO_DEPRECATED"
    )

    if(WIN32)
        set_property(TARGET asio::asio APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES ws2_32 mswsock
        )
    else()
        find_package(Threads REQUIRED)
        set_property(TARGET asio::asio APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES Threads::Threads
        )
    endif()
endif()
