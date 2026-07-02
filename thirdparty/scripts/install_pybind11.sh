#!/usr/bin/env bash
set -euo pipefail

VER_TAG="v2.13.1"
REPO_URL="https://github.com/pybind/pybind11.git"
SRC_DIR="pybind11"

echo "---- Installing pybind11 (${VER_TAG}) ----"

if [ ! -d "${SRC_DIR}" ]; then
    git clone --depth 1 --branch "${VER_TAG}" "${REPO_URL}" "${SRC_DIR}"
else
    cd "${SRC_DIR}"
    git fetch --depth 1 origin "refs/tags/${VER_TAG}:refs/tags/${VER_TAG}" || true
    git checkout "${VER_TAG}"
    cd ..
fi

cmake -S "${SRC_DIR}" -B "${SRC_DIR}/build" \
    ${SHARED_CMAKE_ARGS} \
    -DPYBIND11_TEST=OFF

cmake --build "${SRC_DIR}/build" --target install --config Release -j "${NUM_JOBS}"

echo "Installed pybind11"