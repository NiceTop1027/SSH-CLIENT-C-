#!/bin/bash
# Multi-platform build script for SSH Client
# This script builds the project for all supported platforms

set -e

PROJECT_NAME="ssh-client"
VERSION="1.0.0"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "======================================="
echo "SSH Client Multi-Platform Build Script"
echo "Version: $VERSION"
echo "======================================="

# Detect current platform
PLATFORM=$(uname -s)
echo "Current platform: $PLATFORM"

# Build function
build_platform() {
    local platform=$1
    local build_dir="build_${platform}"

    echo ""
    echo "${YELLOW}Building for ${platform}...${NC}"

    mkdir -p "$build_dir"
    cd "$build_dir"

    # Configure
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="../dist/${platform}"

    # Build
    cmake --build . --config Release --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    # Install
    cmake --install .

    # Run tests
    echo "${YELLOW}Running tests...${NC}"
    ctest --output-on-failure --build-config Release || echo "${RED}Some tests failed${NC}"

    cd ..

    echo "${GREEN}Build complete for ${platform}${NC}"
}

# Package function
package_platform() {
    local platform=$1
    local build_dir="build_${platform}"

    echo ""
    echo "${YELLOW}Creating package for ${platform}...${NC}"

    cd "$build_dir"
    cpack -C Release
    cd ..

    echo "${GREEN}Package created for ${platform}${NC}"
}

# Main execution
case "$PLATFORM" in
    Linux)
        build_platform "linux"
        package_platform "linux"
        ;;
    Darwin)
        build_platform "macos"
        package_platform "macos"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        build_platform "windows"
        package_platform "windows"
        ;;
    *)
        echo "${RED}Unsupported platform: $PLATFORM${NC}"
        exit 1
        ;;
esac

echo ""
echo "${GREEN}All builds complete!${NC}"
echo "Packages available in build_*/packages/"
