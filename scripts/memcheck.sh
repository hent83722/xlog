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
echo "Running memory checks with Valgrind..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel

SUPP_FILE="$SCRIPT_DIR/valgrind_suppressions.supp"
SUPP_OPT=""
if [ -f "$SUPP_FILE" ]; then
    SUPP_OPT="--suppressions=$SUPP_FILE"
fi

TEST_EXE=""
if [ -f ./tests_build/tests ]; then
    TEST_EXE="./tests_build/tests"
elif [ -f ./tests ]; then
    TEST_EXE="./tests"
fi

if [ -n "$TEST_EXE" ]; then
    echo "Running valgrind on $TEST_EXE..."
    valgrind --leak-check=full --show-leak-kinds=all \
             --track-origins=yes \
             $SUPP_OPT \
             "$TEST_EXE"
else
    echo "No test executable found. Building and checking QA tests instead..."
    cd "$PROJECT_DIR/qa-tests"
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --parallel
    valgrind --leak-check=full --show-leak-kinds=all \
             --track-origins=yes \
             $SUPP_OPT \
             ./test_basic_logging
fi
