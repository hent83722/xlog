#!/bin/bash

# Zyrnix v1.1.0 Feature Verification Script
# This script checks which new features are available

echo "========================================"
echo "Zyrnix v1.1.0 Feature Availability Check"
echo "========================================"
echo ""

# Check for zlib (gzip compression)
if pkg-config --exists zlib 2>/dev/null || [ -f /usr/include/zlib.h ]; then
    echo "✓ Gzip compression support: AVAILABLE"
    HAVE_GZIP=1
else
    echo "✗ Gzip compression support: NOT AVAILABLE"
    echo "  Install with: apt-get install zlib1g-dev"
    HAVE_GZIP=0
fi

# Check for zstd
if pkg-config --exists libzstd 2>/dev/null || [ -f /usr/include/zstd.h ]; then
    echo "✓ Zstd compression support: AVAILABLE"
    HAVE_ZSTD=1
else
    echo "✗ Zstd compression support: NOT AVAILABLE"
    echo "  Install with: apt-get install libzstd-dev"
    HAVE_ZSTD=0
fi

# Check for curl/libcurl
if pkg-config --exists libcurl 2>/dev/null || [ -f /usr/include/curl/curl.h ]; then
    echo "✓ Cloud sinks support (libcurl): AVAILABLE"
    HAVE_CURL=1
else
    echo "✗ Cloud sinks support (libcurl): NOT AVAILABLE"
    echo "  Install with: apt-get install libcurl4-openssl-dev"
    HAVE_CURL=0
fi

# Check for curl command
if command -v curl &> /dev/null; then
    echo "✓ Curl command-line tool: AVAILABLE"
    HAVE_CURL_CMD=1
else
    echo "✗ Curl command-line tool: NOT AVAILABLE"
    HAVE_CURL_CMD=0
fi

echo ""
echo "========================================"
echo "New Features Status (v1.1.0)"
echo "========================================"

echo "✓ Rate Limiting & Sampling: BUILT-IN (no dependencies)"
echo "$([ $HAVE_GZIP -eq 1 ] && echo '✓' || echo '✗') Compression (gzip): $([ $HAVE_GZIP -eq 1 ] && echo 'READY' || echo 'NEEDS ZLIB')"
echo "$([ $HAVE_ZSTD -eq 1 ] && echo '✓' || echo '✗') Compression (zstd): $([ $HAVE_ZSTD -eq 1 ] && echo 'READY' || echo 'NEEDS LIBZSTD')"
echo "$([ $HAVE_CURL -eq 1 ] || [ $HAVE_CURL_CMD -eq 1 ]) && echo '✓' || echo '✗') Cloud Sinks: $([ $HAVE_CURL -eq 1 ] && echo 'READY (native)' || ([ $HAVE_CURL_CMD -eq 1 ] && echo 'READY (fallback)' || echo 'NEEDS LIBCURL'))"
echo "✓ Metrics & Observability: BUILT-IN (no dependencies)"

echo ""
echo "========================================"
echo "Build Recommendations"
echo "========================================"

if [ $HAVE_GZIP -eq 0 ] || [ $HAVE_ZSTD -eq 0 ] || [ $HAVE_CURL -eq 0 ]; then
    echo ""
    echo "To enable all features, install missing dependencies:"
    echo ""
    [ $HAVE_GZIP -eq 0 ] && echo "  sudo apt-get install zlib1g-dev"
    [ $HAVE_ZSTD -eq 0 ] && echo "  sudo apt-get install libzstd-dev"
    [ $HAVE_CURL -eq 0 ] && echo "  sudo apt-get install libcurl4-openssl-dev"
    echo ""
    echo "Then rebuild:"
    echo "  cd build && cmake .. && make"
else
    echo ""
    echo "All dependencies available! Build with:"
    echo "  cd build && cmake .. && make"
    echo ""
    echo "All v1.1.0 features will be enabled."
fi

echo ""
echo "For custom builds, use CMake options:"
echo "  cmake -DXLOG_ENABLE_COMPRESSION=OFF .."
echo "  cmake -DXLOG_ENABLE_CLOUD_SINKS=OFF .."
echo "  cmake -DXLOG_MINIMAL=ON .."
echo ""

# Exit with error if critical dependencies are missing
if [ $HAVE_GZIP -eq 0 ] && [ $HAVE_ZSTD -eq 0 ] && [ $HAVE_CURL -eq 0 ]; then
    echo "⚠ Warning: No optional dependencies found."
    echo "  Core features will work, but v1.1.0 features will be limited."
    exit 1
fi

exit 0
