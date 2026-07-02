cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(flexiv_amr_examples
        LANGUAGES CXX
    )
endif()

# When examples are built inside the SDK source tree, flexiv::amr already exists.
# When examples are built as a standalone project, find the installed SDK package.
if(NOT TARGET flexiv::flexiv_amr)
    find_package(flexiv_amr_sdk REQUIRED)
endif()

if(NOT TARGET spdlog::spdlog)
    find_package(spdlog CONFIG REQUIRED)
endif()

function(add_amr_example target_name)
    add_executable(${target_name}
        ${target_name}.cpp
    )

    target_link_libraries(${target_name}
        PRIVATE
            flexiv::flexiv_amr
    )

    target_compile_features(${target_name}
        PRIVATE
            cxx_std_17
    )
endfunction()

add_amr_example(seer_basic_example)
add_amr_example(seer_motion_example)
add_amr_example(seer_navigation_example)
add_amr_example(seer_audio_example)
