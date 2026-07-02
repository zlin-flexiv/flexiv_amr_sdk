# Flexiv AMR SDK

Flexiv AMR SDK provides C++ and Python APIs for AMR control and status query.

This repository is the public binary distribution of Flexiv AMR SDK. It contains public headers, examples, CMake integration files, and SHA256 files for prebuilt libraries. The actual prebuilt static libraries are distributed through GitHub Release Assets.

## Supported Platforms

| OS | Architecture | Compiler |
|---|---|---|
| Ubuntu 20.04+ | x86_64 | GCC |
| Ubuntu 20.04+ | aarch64 | GCC |
| macOS 12+ | arm64 | Apple Clang |
| Windows 10+ | x86_64 | MSVC 2022 |

## Repository Layout

```text
flexiv_amr_sdk/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── cmake/
│   ├── FlexivInstallLibrary.cmake
│   ├── Findasio.cmake
│   └── flexiv_amr_sdk-config.cmake.in
├── include/
│   └── flexiv/amr/
├── example/
│   ├── CMakeLists.txt
│   ├── seer_audio_example.cpp
│   ├── seer_basic_example.cpp
│   ├── seer_motion_example.cpp
│   └── seer_navigation_example.cpp
├── example_py/
│   ├── seer_audio_example.py
│   ├── seer_basic_example.py
│   ├── seer_motion_example.py
│   └── seer_navigation_example.py
├── thirdparty/
│   └── build_and_install_dependencies.sh
├── lib/
│   ├── dummy.cpp
│   ├── libflexiv_amr.x86_64-linux-gnu.a.sha256
│   ├── libflexiv_amr.aarch64-linux-gnu.a.sha256
│   ├── libflexiv_amr.arm64-darwin.a.sha256
│   └── flexiv_amr.win_amd64.lib.sha256
└── python/
    └── README.md
```

## Binary Libraries

The prebuilt C++ static libraries are not stored in this repository. They are distributed through GitHub Release Assets.

During CMake configuration, the SDK automatically selects the matching library for the current platform, downloads it from the corresponding GitHub Release, and verifies it with the SHA256 file stored under `lib/`.

The downloaded library is then installed as a standard CMake package target:

```cmake
flexiv::flexiv_amr
```

## Install Dependencies

Install third-party dependencies into a local prefix:

```bash
bash thirdparty/build_and_install_dependencies.sh "$HOME/flexiv_amr_sdk_install" 8
```

This installs dependencies required by the SDK, such as:

```text
asio
nlohmann_json
spdlog
pybind11
```

`spdlog` is mainly required by the example programs. `pybind11` is only required if you use Python-related builds or examples.

## Build And Install SDK

Configure the SDK:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/flexiv_amr_sdk_install" \
  -DCMAKE_PREFIX_PATH="$HOME/flexiv_amr_sdk_install"
```

Build and install:

```bash
cmake --build build --config Release --parallel 8
cmake --install build --config Release
```

After installation, the package can be found through:

```bash
-DCMAKE_PREFIX_PATH="$HOME/flexiv_amr_sdk_install"
```

## Use In Your CMake Project

A minimal `CMakeLists.txt` example:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_amr_app
    LANGUAGES CXX
)

find_package(flexiv_amr_sdk REQUIRED)

add_executable(my_amr_app
    main.cpp
)

target_link_libraries(my_amr_app
    PRIVATE
        flexiv::flexiv_amr
)

target_compile_features(my_amr_app
    PRIVATE
        cxx_std_17
)
```

Configure and build your project:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$HOME/flexiv_amr_sdk_install"

cmake --build build --config Release --parallel 8
```

## Build C++ Examples

After installing dependencies and the SDK, build the examples with:

```bash
cmake -S example -B build-example \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$HOME/flexiv_amr_sdk_install"

cmake --build build-example --config Release --parallel 8
```

Example programs:

```text
seer_basic_example
seer_motion_example
seer_navigation_example
seer_audio_example
```

Run an example:

```bash
./build-example/seer_basic_example <amr_ip>
```

For example:

```bash
./build-example/seer_basic_example 192.168.192.5
```

## Python

Python wheels are distributed through GitHub Release Assets.

Supported Python versions:

| Platform | Python |
|---|---|
| Linux x86_64 | 3.10, 3.12 |
| Linux aarch64 | 3.10, 3.12 |
| macOS arm64 | 3.10, 3.12 |
| Windows x86_64 | 3.10, 3.12 |

See:

```text
python/README.md
```

## Notes For Shared Network Access

Some examples communicate directly with the AMR over the network. Make sure that:

```text
1. The host computer and AMR are reachable from each other.
2. The correct AMR IP address is used.
3. Required AMR service ports are not blocked by firewall rules.
4. The AMR is in a safe state before running motion-related examples.
```

## Safety Notice

Motion-related APIs may cause the AMR to move. Before running motion examples, ensure that the robot is in an open and safe environment, and that emergency stop measures are available.

## License

See `LICENSE`.
