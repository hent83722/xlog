#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "██╗  ██╗██╗      ██████╗  ██████╗ "
echo "╚██╗██╔╝██║     ██╔═══██╗██╔════╝ "
echo " ╚███╔╝ ██║     ██║   ██║██║      "
echo " ██╔██╗ ██║     ██║   ██║██║   ██║"
echo "██╔╝ ██╗███████╗╚██████╔╝╚██████╔╝"
echo "╚═╝  ╚═╝╚══════╝ ╚═════╝  ╚═════╝ "

cd "$PROJECT_DIR"
echo "Building and running tests (Debug)..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
add_subdirectory(../tests tests_build)
cmake --build . --parallel

echo ""
echo "Running unit tests..."
if [ -f ./tests_build/tests ]; then
    ./tests_build/tests
elif [ -f ./tests ]; then
    ./tests
else
    echo "No test executable found. Running ctest instead..."
    ctest --output-on-failure
fi

echo ""
echo "Running QA tests..."
if [ -d "$PROJECT_DIR/qa-tests" ]; then
    cd "$PROJECT_DIR/qa-tests"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --parallel
    ./run_all_qa_tests
fi
