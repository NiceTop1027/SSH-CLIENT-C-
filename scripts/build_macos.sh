#!/bin/bash
# macOS build script for SSH Client

set -e

PROJECT_NAME="ssh-client"
BUILD_DIR="build_macos"
INSTALL_DIR="dist/macos"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "======================================="
echo "SSH Client macOS Build Script"
echo "======================================="

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "${RED}Homebrew not found! Please install Homebrew first.${NC}"
    echo "Visit: https://brew.sh"
    exit 1
fi

# Check for required dependencies
echo "${YELLOW}Checking dependencies...${NC}"

if ! brew list qt &> /dev/null; then
    echo "${YELLOW}Installing Qt...${NC}"
    brew install qt
fi

if ! brew list libssh &> /dev/null; then
    echo "${YELLOW}Installing libssh...${NC}"
    brew install libssh
fi

# Set Qt path
if [ -d "/opt/homebrew/opt/qt" ]; then
    export Qt6_DIR="/opt/homebrew/opt/qt/lib/cmake/Qt6"
elif [ -d "/usr/local/opt/qt" ]; then
    export Qt6_DIR="/usr/local/opt/qt/lib/cmake/Qt6"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "${YELLOW}Configuring project...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="../${INSTALL_DIR}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0

if [ $? -ne 0 ]; then
    echo "${RED}CMake configuration failed!${NC}"
    cd ..
    exit 1
fi

echo ""
echo "${YELLOW}Building project...${NC}"
cmake --build . --config Release --parallel $(sysctl -n hw.ncpu)

if [ $? -ne 0 ]; then
    echo "${RED}Build failed!${NC}"
    cd ..
    exit 1
fi

echo ""
echo "${YELLOW}Running tests...${NC}"
ctest -C Release --output-on-failure || echo "${YELLOW}Some tests failed${NC}"

echo ""
echo "${YELLOW}Installing...${NC}"
cmake --install . --config Release

echo ""
echo "${YELLOW}Creating app bundle...${NC}"

# Fix bundle dependencies using macdeployqt
if command -v macdeployqt &> /dev/null; then
    macdeployqt "${PROJECT_NAME}.app" -dmg
    echo "${GREEN}DMG created: ${PROJECT_NAME}.dmg${NC}"
elif command -v Qt6/bin/macdeployqt &> /dev/null; then
    Qt6/bin/macdeployqt "${PROJECT_NAME}.app" -dmg
    echo "${GREEN}DMG created: ${PROJECT_NAME}.dmg${NC}"
else
    echo "${YELLOW}macdeployqt not found - app bundle may not be portable${NC}"
fi

cd ..

echo ""
echo "${GREEN}=======================================${NC}"
echo "${GREEN}Build complete!${NC}"
echo "App bundle: ${BUILD_DIR}/${PROJECT_NAME}.app"
if [ -f "${BUILD_DIR}/${PROJECT_NAME}.dmg" ]; then
    echo "DMG installer: ${BUILD_DIR}/${PROJECT_NAME}.dmg"
fi
echo "${GREEN}=======================================${NC}"
