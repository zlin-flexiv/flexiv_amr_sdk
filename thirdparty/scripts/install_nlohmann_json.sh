#!/usr/bin/env bash
set -euo pipefail

VER_TAG="v3.11.3"
REPO_URL="https://github.com/nlohmann/json.git"
SRC_DIR="json"

echo "---- Installing nlohmann_json (${VER_TAG}) ----"

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
    -DJSON_BuildTests=OFF \
    -DJSON_Install=ON

cmake --build "${SRC_DIR}/build" --target install --config Release -j "${NUM_JOBS}"

echo "Installed nlohmann_json"