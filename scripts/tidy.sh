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
echo "Running static analysis with clang-tidy..."
mkdir -p build
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd ..

COMPILE_DB="-p build"

FILES=$(find src -name '*.cpp' 2>/dev/null || true)

if [ -z "$FILES" ]; then
    echo "No source files found to analyze."
    exit 0
fi

FAILED=0
for f in $FILES; do
    echo "  Analyzing: $f"
    if ! clang-tidy $COMPILE_DB "$f" 2>/dev/null; then
        FAILED=$((FAILED + 1))
    fi
done

if [ $FAILED -gt 0 ]; then
    echo "Static analysis found issues in $FAILED file(s)."
    exit 1
fi

echo "Static analysis complete!"
