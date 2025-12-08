#!/bin/bash

set -e

echo "================================"
echo "Building XLog Demo Application"
echo "================================"
echo ""

# Create build directory
mkdir -p build
cd build

# Check if XLog library exists
XLOG_LIB="../xlog/build/libxlog.a"
if [ ! -f "$XLOG_LIB" ]; then
    echo "❌ XLog library not found at $XLOG_LIB"
    echo "Please build XLog first:"
    echo "  cd ../xlog && mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

echo "✓ Found XLog library"
echo ""

# Configure and build
echo "Configuring..."
cmake ..

echo ""
echo "Building..."
make -j$(nproc)

echo ""
echo "================================"
echo "✓ Build complete!"
echo "================================"
echo ""
echo "Run the demo with:"
echo "  ./build/taskmanager"
echo ""
