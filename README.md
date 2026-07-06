# Flexiv AMR SDK

![Cpp Badge](https://github.com/flexivrobotics/flexiv_amr_sdk/actions/workflows/public_sdk_install_test.yml/badge.svg)
![Python Badge](https://github.com/flexivrobotics/flexiv_amr_sdk/actions/workflows/public_sdk_install_test.yml/badge.svg)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0.html)

Flexiv AMR SDK is a C++ and Python development toolkit that enables users to create AMR applications using APIs for AMR status query, navigation, motion, audio, sensor data, and configuration. The current public release provides SEER AMR integration. Flexiv AMR support is reserved for future releases.

This repository is the public binary distribution of Flexiv AMR SDK. It contains public headers, examples, CMake integration files, dependency install scripts, and SHA256 files for prebuilt libraries. The actual C++ static libraries and Python wheels are distributed through GitHub Release Assets.

## References

- [Flexiv AMR SDK API Documentation](https://flexiv-amr-sdk-doc.netlify.app/) is the main API reference. 

- [Flexiv AMR SDK Releases](https://github.com/flexivrobotics/flexiv_amr_sdk/releases) is the main place to download prebuilt C++ static libraries and Python wheels. The instructions below serve as a quick reference.

## Environment Compatibility

| **OS**                | **Platform**    | **C++ compiler kit** | **Python interpreter** |
| --------------------- | --------------- | -------------------- | ---------------------- |
| Linux (Ubuntu 20.04+) | x86_64, aarch64 | GCC                  | 3.10, 3.12             |
| macOS 12+             | arm64           | Clang                | 3.10, 3.12             |
| Windows 10+           | x86_64          | MSVC  v14.3+         | 3.10, 3.12             |

## Important Notice

Before trying to run any AMR SDK program, please make sure the host computer can reach the AMR over the network, the correct AMR IP address is used, and required service ports are not blocked by firewall rules. Motion-related APIs can cause the AMR to move, so run navigation and motion examples only in a safe environment with emergency stop measures available.

## Quick Start - Python

### Install the Python package

On all supported platforms, download the Python wheel that matches your OS, architecture, and Python version from [GitHub Releases](https://github.com/flexivrobotics/flexiv_amr_sdk/releases), then install it using the `pip` module:

    python3.x -m pip install ./flexiv_amr-*.whl

> [!NOTE]
> Replace `3.x` with a specific Python version, such as `3.10` or `3.12`.

### Use the installed Python package

After the `flexiv_amr` Python package is installed, it can be imported from any Python script. Test with the following commands in a new Terminal:

    python3.x
    import flexiv_amr
    options = flexiv_amr.SeerAmrOptions()
    options.host = "192.168.192.5"
    amr = flexiv_amr.SeerAmrClient(options)

The program will try to create a SEER AMR client with IP address `192.168.192.5`. Replace this IP address with the actual AMR IP address in your network.

### Run example Python scripts

To run an example Python script in this repo:

    cd flexiv_amr_sdk/example_py
    python3.x <example-name>.py <amr-ip>

For example:

    python3.10 ./seer_basic_example.py 192.168.192.5

Available Python examples:

    seer_basic_example.py
    seer_motion_example.py
    seer_navigation_example.py
    seer_audio_example.py

## Quick Start - C++

### Prepare build tools

#### Linux

1. Install compiler kit using package manager:

       sudo apt install build-essential

2. Install CMake using package manager:

       sudo apt install cmake

#### macOS

1. Install compiler kit using `xcode` tool:

       xcode-select --install

   This will invoke the installation of Xcode Command Line Tools, then follow the prompted window to finish the installation.

2. Install CMake using package manager:

       brew install cmake

#### Windows

1. Install compiler kit: Download and install Microsoft Visual Studio 2019 (MSVC v14.2) or above. Choose "Desktop development with C++" under the *Workloads* tab during installation. You only need to keep the following components for the selected workload:
   * MSVC ... C++ x64/x86 build tools (Latest)
   * C++ CMake tools for Windows
   * Windows 10 SDK or Windows 11 SDK, depending on your actual Windows version
2. Install CMake: Download `cmake-3.x.x-windows-x86_64.msi` from [CMake download page](https://cmake.org/download/) and install the msi file. The minimum required version is 3.20. **Add CMake to system PATH** when prompted, so that `cmake` and `cmake-gui` command can be used from Command Prompt or a bash emulator.
3. Install bash emulator: Download and install [Git for Windows](https://git-scm.com/install/windows), which comes with a bash emulator Git Bash. The following steps are to be carried out in this bash emulator.

### Install the C++ library

The following steps are mostly the same on all supported platforms, with some variations.

1. Choose a directory for installing the C++ library of AMR SDK and its dependencies. This directory can be under system path or not, depending on whether you want AMR SDK to be globally discoverable by CMake. For example, a new folder named `flexiv_amr_sdk_install` under the home directory.
2. In a new Terminal, run the provided script to compile and install all dependencies to the installation directory chosen in step 1:

       cd flexiv_amr_sdk
       bash thirdparty/build_and_install_dependencies.sh ~/flexiv_amr_sdk_install 8

3. In the same Terminal, configure the `flexiv_amr_sdk` CMake project:

       cd flexiv_amr_sdk
       mkdir build && cd build
       cmake .. -DCMAKE_INSTALL_PREFIX=~/flexiv_amr_sdk_install -DCMAKE_PREFIX_PATH=~/flexiv_amr_sdk_install

   > [!NOTE]
   > `-D` followed by `CMAKE_INSTALL_PREFIX` sets the absolute path of the installation directory, which should be the one chosen in step 1. `CMAKE_PREFIX_PATH` tells CMake where to find dependencies installed by `thirdparty/build_and_install_dependencies.sh`.

4. Install `flexiv_amr_sdk` C++ library to `CMAKE_INSTALL_PREFIX` path, which may or may not be globally discoverable by CMake depending on the location:

       cd flexiv_amr_sdk/build
       cmake --build . --target install --config Release

### Use the installed C++ library

After the library is installed as the `flexiv_amr_sdk` CMake package, it can be linked from any other CMake projects. Using the provided `example` project for instance:

    cd flexiv_amr_sdk/example
    mkdir build && cd build
    cmake .. -DCMAKE_PREFIX_PATH=~/flexiv_amr_sdk_install
    cmake --build . --config Release -j 4

> [!NOTE]
> `-D` followed by `CMAKE_PREFIX_PATH` tells the user project's CMake where to find the installed C++ library. This argument can be skipped if the AMR SDK library and its dependencies are installed to a globally discoverable location.

### Run example C++ programs

The steps to run an example C++ program compiled during the previous step vary by OS.

> [!NOTE]
> - Replace `<example-name>` with the actual example program to be executed.
> - Replace `<amr-ip>` with the actual IP address of the AMR, for example `192.168.192.5`.
> - Be careful when running motion or navigation examples because they can move the AMR.

#### Linux and macOS

On UNIX systems, run the example program from the example build directory:

    cd flexiv_amr_sdk/example/build
    ./<example-name> <amr-ip>

For example:

    ./seer_basic_example 192.168.192.5

#### Windows - Command Prompt

Run the example program from the example build directory:

    cd flexiv_amr_sdk\example\build
    Release\<example-name>.exe <amr-ip>

For example:

    Release\seer_basic_example.exe 192.168.192.5

#### Windows - bash emulator (such as Git Bash)

The same rule applies in a bash emulator, but using bash path syntax:

    cd flexiv_amr_sdk/example/build
    ./Release/<example-name>.exe <amr-ip>

Available C++ examples:

    seer_basic_example
    seer_motion_example
    seer_navigation_example
    seer_audio_example
