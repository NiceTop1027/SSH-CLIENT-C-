#!/bin/bash
# Linux build script for SSH Client

set -e

PROJECT_NAME="ssh-client"
BUILD_DIR="build_linux"
INSTALL_DIR="dist/linux"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "======================================="
echo "SSH Client Linux Build Script"
echo "======================================="

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
    echo "Detected distribution: $DISTRO"
else
    DISTRO="unknown"
fi

# Check and install dependencies
check_dependencies() {
    echo "${YELLOW}Checking dependencies...${NC}"

    local missing_deps=()

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if ! command -v qmake &> /dev/null && ! command -v qmake-qt6 &> /dev/null; then
        missing_deps+=("qt6" "qtbase6-dev")
    fi

    if ! pkg-config --exists libssh; then
        missing_deps+=("libssh-dev")
    fi

    if [ ${#missing_deps[@]} -gt 0 ]; then
        echo "${RED}Missing dependencies: ${missing_deps[*]}${NC}"
        echo ""
        echo "Install with:"

        case "$DISTRO" in
            ubuntu|debian)
                echo "  sudo apt-get update"
                echo "  sudo apt-get install cmake qt6-base-dev libssh-dev build-essential"
                ;;
            fedora|rhel|centos)
                echo "  sudo dnf install cmake qt6-qtbase-devel libssh-devel gcc-c++"
                ;;
            arch)
                echo "  sudo pacman -S cmake qt6-base libssh base-devel"
                ;;
            opensuse*)
                echo "  sudo zypper install cmake qt6-base-devel libssh-devel gcc-c++"
                ;;
            *)
                echo "  Please install: cmake, Qt6, libssh, and build tools"
                ;;
        esac

        return 1
    fi

    echo "${GREEN}All dependencies satisfied${NC}"
    return 0
}

if ! check_dependencies; then
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "${YELLOW}Configuring project...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="../${INSTALL_DIR}"

if [ $? -ne 0 ]; then
    echo "${RED}CMake configuration failed!${NC}"
    cd ..
    exit 1
fi

echo ""
echo "${YELLOW}Building project...${NC}"
cmake --build . --config Release --parallel $(nproc)

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
echo "${YELLOW}Creating packages...${NC}"

# Create DEB package
cpack -G DEB || echo "${YELLOW}DEB package creation failed${NC}"

# Create RPM package
cpack -G RPM || echo "${YELLOW}RPM package creation failed${NC}"

# Create tarball
cpack -G TGZ || echo "${YELLOW}TGZ package creation failed${NC}"

cd ..

echo ""
echo "${GREEN}=======================================${NC}"
echo "${GREEN}Build complete!${NC}"
echo "Binary: ${BUILD_DIR}/${PROJECT_NAME}"
echo "Install dir: ${INSTALL_DIR}/"
if ls ${BUILD_DIR}/*.deb 2>/dev/null; then
    echo "DEB package: $(ls ${BUILD_DIR}/*.deb)"
fi
if ls ${BUILD_DIR}/*.rpm 2>/dev/null; then
    echo "RPM package: $(ls ${BUILD_DIR}/*.rpm)"
fi
if ls ${BUILD_DIR}/*.tar.gz 2>/dev/null; then
    echo "Tarball: $(ls ${BUILD_DIR}/*.tar.gz)"
fi
echo "${GREEN}=======================================${NC}"
