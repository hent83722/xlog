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
echo "Formatting source files..."

FILES=$(find include src examples tests benchmarks qa-tests -name '*.hpp' -o -name '*.cpp' 2>/dev/null || true)

if [ -z "$FILES" ]; then
    echo "No source files found to format."
    exit 0
fi

for f in $FILES; do
    echo "  Formatting: $f"
    clang-format -i "$f"
done

echo "Formatting complete!"
