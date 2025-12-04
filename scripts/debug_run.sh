#!/usr/bin/env bash
set -e

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel


if [ -f ./test ]; then
    ./test
else
    echo "No executable 'test' found. Make sure you have a test program."
fi
