#!/usr/bin/env bash
# Build and install all dependencies of flexiv_amr_sdk
echo ">>>>> Start: flexiv_amr_sdk/thirdparty/build_and_install_dependencies.sh <<<<<"

# Absolute path of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
set -euo pipefail

# Check script arguments
if [ "$#" -lt 1 ]; then 
    echo "Usage: $0 <install-prefix> [num-jobs]"
    echo "Example:"
    echo "  bash build_and_install_dependencies.sh ~/flexiv_amr_sdk_install 4"
    exit 1
fi 

# Get dependencies install directory from script argument
# should be the same as the install directory of flexiv_amr_sdk
export INSTALL_DIR="$(mkdir -p "$1" && cd "$1" && pwd)"
echo "Dependencies will be installed toL $INSTALL_DIR"

# Use specified number for parallel build jobs, otherwise use 4
export NUM_JOBS="${2:-4}"
echo "Number of parallel build jobs: $NUM_JOBS"

# Set shared cmake arguments
export SHARED_CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release \
                          -DBUILD_SHARED_LIBS=OFF \
                          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
                          -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
                          -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
                          -DBUILD_TESTING=OFF"

if [ -n "${AMR_CMAKE_TOOLCHAIN_FILE:-}" ]; then
    export SHARED_CMAKE_ARGS="${SHARED_CMAKE_ARGS} \
                              -DCMAKE_TOOLCHAIN_FILE=${AMR_CMAKE_TOOLCHAIN_FILE}"
    echo "Using CMake toolchain file: ${AMR_CMAKE_TOOLCHAIN_FILE}"
fi

# Clone all dependencies in a subfolder
mkdir -p "${SCRIPT_DIR}/cloned"
cd "${SCRIPT_DIR}/cloned"

# Build and install all dependencies to INSTALL_DIR
bash "${SCRIPT_DIR}/scripts/install_asio.sh"
bash "${SCRIPT_DIR}/scripts/install_nlohmann_json.sh"
bash "${SCRIPT_DIR}/scripts/install_spdlog.sh"

if [ "${AMR_BUILD_PYBIND11:-ON}" = "ON" ]; then
    bash "${SCRIPT_DIR}/scripts/install_pybind11.sh"
else
    echo "Skipping pybind11 installation because AMR_BUILD_PYBIND11=${AMR_BUILD_PYBIND11}"
fi

echo ">>>>>>>>>> Finished <<<<<<<<<<"
