#!/usr/bin/env bash
set -e

FILES=$(find include src examples tests -name '*.hpp' -o -name '*.cpp')
for f in $FILES; do
    clang-format -i "$f"
done

echo "Formatting complete!"
