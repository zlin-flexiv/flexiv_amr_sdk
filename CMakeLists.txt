cmake_minimum_required(VERSION 3.20)

project(flexiv_amr_sdk
    VERSION 1.0.0
    DESCRIPTION "Flexiv AMR SDK"
    LANGUAGES CXX
)

set(FLEXIV_AMR_TARGET_NAME flexiv_amr)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "CMake build type" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Release" "Debug" "RelWithDebInfo" "MinSizeRel"
)

message(STATUS "OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(FLEXIV_AMR_LIB "libflexiv_amr.x86_64-linux-gnu.a")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(FLEXIV_AMR_LIB "libflexiv_amr.aarch64-linux-gnu.a")
    else()
        message(FATAL_ERROR "Linux with ${CMAKE_SYSTEM_PROCESSOR} processor is not supported.")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(FLEXIV_AMR_LIB "libflexiv_amr.arm64-darwin.a")
    else()
        message(FATAL_ERROR "macOS with ${CMAKE_SYSTEM_PROCESSOR} processor is not supported.")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(FLEXIV_AMR_LIB "flexiv_amr.win_amd64.lib")
    else()
        message(FATAL_ERROR "Windows with ${CMAKE_SYSTEM_PROCESSOR} processor is not supported.")
    endif()
else()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported.")
endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/lib/${FLEXIV_AMR_LIB}.sha256" EXPECTED_HASH)
string(STRIP "${EXPECTED_HASH}" EXPECTED_HASH)

set(FLEXIV_AMR_LIB_URL
    "https://github.com/zlin-flexiv/flexiv_amr_sdk/releases/download/v${PROJECT_VERSION}/${FLEXIV_AMR_LIB}"
)

message(STATUS "Downloading ${FLEXIV_AMR_LIB_URL} ...")

file(DOWNLOAD
    "${FLEXIV_AMR_LIB_URL}"
    "${CMAKE_CURRENT_BINARY_DIR}/${FLEXIV_AMR_LIB}"
    SHOW_PROGRESS
    TLS_VERIFY ON
    EXPECTED_HASH SHA256=${EXPECTED_HASH}
    STATUS download_status
)

list(GET download_status 0 download_code)
list(GET download_status 1 download_message)

if(download_code EQUAL 0)
    message(STATUS "Download finished")
else()
    message(FATAL_ERROR "Download failed: ${download_message}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# PROJECT DEPENDENCIES
# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Standalone asio
find_package(asio REQUIRED)

# nlohmann json
find_package(nlohmann_json CONFIG REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# CREATE LIBRARY
# Create static library target with dummy source
add_library(${FLEXIV_AMR_TARGET_NAME} STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/dummy.cpp"
)

# Create an alias of the target with flexiv namespace
add_library(flexiv::flexiv_amr ALIAS ${FLEXIV_AMR_TARGET_NAME})

# Set include directories
target_include_directories(${FLEXIV_AMR_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link to dependencies
target_link_libraries(${FLEXIV_AMR_TARGET_NAME}
    PUBLIC
        Threads::Threads
        asio::asio
        nlohmann_json::nlohmann_json
)

target_compile_features(${FLEXIV_AMR_TARGET_NAME}
    PUBLIC
        cxx_std_17
)

if(MSVC)
    target_compile_definitions(${FLEXIV_AMR_TARGET_NAME}
        PUBLIC
            _WIN32_WINNT=0x0A00
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
endif()

option(FLEXIV_AMR_BUILD_EXAMPLES "Build Flexiv AMR SDK examples" OFF)

if(FLEXIV_AMR_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FlexivInstallLibrary.cmake")
FlexivInstallLibrary()
