#!/usr/bin/env bash
set -euo pipefail

VER_TAG="asio-1-36-0"
REPO_URL="https://github.com/chriskohlhoff/asio.git"
SRC_DIR="asio"

echo "---- Installing standalone Asio (${VER_TAG}) ----"

if [ ! -d "${SRC_DIR}" ]; then
    git clone --depth 1 --branch "${VER_TAG}" "${REPO_URL}" "${SRC_DIR}"
else
    cd "${SRC_DIR}"
    git fetch --depth 1 origin "refs/tags/${VER_TAG}:refs/tags/${VER_TAG}" || true
    git checkout "${VER_TAG}"
    cd ..
fi 

mkdir -p "${INSTALL_DIR}/include"

if command -v rsync >/dev/null 2>&1; then
    rsync -a --delete "${SRC_DIR}/asio/include/" "${INSTALL_DIR}/include/"
else
    cp -R "${SRC_DIR}/asio/include/." "${INSTALL_DIR}/include/"
fi

echo "Installed asio"