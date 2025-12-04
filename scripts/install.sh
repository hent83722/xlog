#!/usr/bin/env bash
set -e

bash scripts/build.sh
cd build
sudo cmake --install .
echo "XLog installed to system directories!"
