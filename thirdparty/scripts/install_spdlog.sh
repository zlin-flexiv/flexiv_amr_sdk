#!/usr/bin/env bash
set -euo pipefail

VER_TAG="v1.14.1"
REPO_URL="https://github.com/gabime/spdlog.git"
SRC_DIR="spdlog"

echo "---- Installing spdlog (${VER_TAG}) ----"

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
    -DSPDLOG_BUILD_EXAMPLE=OFF \
    -DSPDLOG_BUILD_TESTS=OFF \
    -DSPDLOG_BUILD_BENCH=OFF

cmake --build "${SRC_DIR}/build" --target install --config Release -j "${NUM_JOBS}"

echo "Installed spdlog"