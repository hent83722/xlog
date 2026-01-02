#!/usr/bin/env bash
#
# Zyrnix Installation Script for macOS
# Builds from source and installs system-wide
#
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}"
echo "██╗  ██╗██╗      ██████╗  ██████╗ "
echo "╚██╗██╔╝██║     ██╔═══██╗██╔════╝ "
echo " ╚███╔╝ ██║     ██║   ██║██║      "
echo " ██╔██╗ ██║     ██║   ██║██║   ██║"
echo "██╔╝ ██╗███████╗╚██████╔╝╚██████╔╝"
echo "╚═╝  ╚═╝╚══════╝ ╚═════╝  ╚═════╝ "
echo -e "${NC}"
echo -e "${GREEN}Zyrnix Installation Script for macOS${NC}"
echo "=========================================="

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# Check for required tools
echo -e "${YELLOW}Checking dependencies...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake is not installed.${NC}"
    echo "Install with Homebrew: brew install cmake"
    echo "Or download from: https://cmake.org/download/"
    exit 1
fi

if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: Xcode Command Line Tools not installed.${NC}"
    echo "Install with: xcode-select --install"
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"

# Parse command line arguments
BUILD_TYPE="Release"
INSTALL_PREFIX="/usr/local"
PARALLEL_JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --prefix=*)
            INSTALL_PREFIX="${1#*=}"
            shift
            ;;
        --jobs=*)
            PARALLEL_JOBS="${1#*=}"
            shift
            ;;
        --homebrew)
            # Use Homebrew's prefix for consistency
            INSTALL_PREFIX="$(brew --prefix 2>/dev/null || echo /usr/local)"
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug          Build in debug mode (default: Release)"
            echo "  --prefix=PATH    Installation prefix (default: /usr/local)"
            echo "  --homebrew       Use Homebrew's prefix for installation"
            echo "  --jobs=N         Number of parallel build jobs (default: auto)"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo -e "${YELLOW}Configuration:${NC}"
echo "  Build Type:      $BUILD_TYPE"
echo "  Install Prefix:  $INSTALL_PREFIX"
echo "  Parallel Jobs:   $PARALLEL_JOBS"
echo ""

# Create and enter build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"

# Build
echo -e "${YELLOW}Building Zyrnix...${NC}"
cmake --build . --parallel "$PARALLEL_JOBS"

echo -e "${GREEN}Build complete!${NC}"
echo ""

# Install
echo -e "${YELLOW}Installing Zyrnix (requires sudo)...${NC}"
sudo cmake --install .

echo ""
echo -e "${GREEN}=========================================="
echo "Zyrnix has been installed successfully!"
echo "==========================================${NC}"
echo ""
echo "Installation location: $INSTALL_PREFIX"
echo ""
echo "To use Zyrnix in your CMake project, add:"
echo "  find_package(Zyrnix REQUIRED)"
echo "  target_link_libraries(your_target PRIVATE Zyrnix::Zyrnix)"
echo ""
