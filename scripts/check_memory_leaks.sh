#!/bin/bash
# Memory leak detection script for SSH Client
# Supports multiple tools: Valgrind (Linux), AddressSanitizer, and macOS leaks

set -e

BUILD_DIR="${BUILD_DIR:-build}"
PROJECT_NAME="ssh-client"
REPORT_DIR="memory_reports"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "====================================="
echo "Memory Leak Detection for SSH Client"
echo "====================================="

# Create report directory
mkdir -p "$REPORT_DIR"

# Detect platform
PLATFORM=$(uname -s)
echo "Platform: $PLATFORM"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to run with AddressSanitizer
run_asan() {
    echo ""
    echo "${YELLOW}Running with AddressSanitizer...${NC}"

    # Rebuild with AddressSanitizer
    ASAN_BUILD_DIR="${BUILD_DIR}_asan"
    mkdir -p "$ASAN_BUILD_DIR"
    cd "$ASAN_BUILD_DIR"

    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"

    cmake --build . --config Debug

    # Run tests
    export ASAN_OPTIONS="detect_leaks=1:symbolize=1:log_path=../${REPORT_DIR}/asan"

    if [ -f "./ssh-client" ]; then
        echo "Running application with AddressSanitizer..."
        timeout 5 ./ssh-client || true
    fi

    if [ -d "./tests" ]; then
        echo "Running tests with AddressSanitizer..."
        ctest --output-on-failure || true
    fi

    cd ..

    # Check for reports
    if ls "${REPORT_DIR}"/asan.* 1> /dev/null 2>&1; then
        echo "${RED}Memory issues detected! Check reports in ${REPORT_DIR}${NC}"
        return 1
    else
        echo "${GREEN}No memory leaks detected by AddressSanitizer${NC}"
        return 0
    fi
}

# Function to run with Valgrind (Linux)
run_valgrind() {
    echo ""
    echo "${YELLOW}Running with Valgrind...${NC}"

    if ! command_exists valgrind; then
        echo "${RED}Valgrind not found. Install with: sudo apt-get install valgrind${NC}"
        return 1
    fi

    # Ensure debug build exists
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake .. -DCMAKE_BUILD_TYPE=Debug
        cmake --build .
        cd ..
    fi

    VALGRIND_REPORT="${REPORT_DIR}/valgrind_report.txt"

    if [ -f "${BUILD_DIR}/ssh-client" ]; then
        echo "Running application with Valgrind..."
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --verbose \
            --log-file="$VALGRIND_REPORT" \
            timeout 5 "${BUILD_DIR}/ssh-client" || true
    fi

    # Run tests
    if [ -d "${BUILD_DIR}/tests" ]; then
        echo "Running tests with Valgrind..."
        cd "${BUILD_DIR}"
        for test in tests/unit/test_* tests/integration/test_*; do
            if [ -f "$test" ]; then
                test_name=$(basename "$test")
                echo "  Testing: $test_name"
                valgrind \
                    --leak-check=full \
                    --show-leak-kinds=all \
                    --log-file="../${REPORT_DIR}/valgrind_${test_name}.txt" \
                    "$test" || true
            fi
        done
        cd ..
    fi

    # Parse results
    echo ""
    echo "Valgrind Results:"
    if grep -q "ERROR SUMMARY: 0 errors" "${REPORT_DIR}"/valgrind_*.txt 2>/dev/null; then
        echo "${GREEN}No memory errors detected${NC}"
        return 0
    else
        echo "${RED}Memory errors detected! Check reports in ${REPORT_DIR}${NC}"
        grep "ERROR SUMMARY" "${REPORT_DIR}"/valgrind_*.txt || true
        return 1
    fi
}

# Function to run macOS leaks tool
run_macos_leaks() {
    echo ""
    echo "${YELLOW}Running macOS leaks tool...${NC}"

    if ! command_exists leaks; then
        echo "${RED}leaks command not found${NC}"
        return 1
    fi

    # Ensure build exists
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake .. -DCMAKE_BUILD_TYPE=Debug
        cmake --build .
        cd ..
    fi

    if [ -f "${BUILD_DIR}/ssh-client.app/Contents/MacOS/ssh-client" ]; then
        APP_PATH="${BUILD_DIR}/ssh-client.app/Contents/MacOS/ssh-client"
    elif [ -f "${BUILD_DIR}/ssh-client" ]; then
        APP_PATH="${BUILD_DIR}/ssh-client"
    else
        echo "${RED}Application not found${NC}"
        return 1
    fi

    # Run application in background
    "$APP_PATH" &
    APP_PID=$!
    sleep 3

    # Check for leaks
    LEAKS_REPORT="${REPORT_DIR}/macos_leaks.txt"
    leaks "$APP_PID" > "$LEAKS_REPORT" 2>&1

    # Kill application
    kill "$APP_PID" 2>/dev/null || true

    # Parse results
    if grep -q "0 leaks for 0 total leaked bytes" "$LEAKS_REPORT"; then
        echo "${GREEN}No memory leaks detected${NC}"
        return 0
    else
        echo "${RED}Memory leaks detected! Check $LEAKS_REPORT${NC}"
        return 1
    fi
}

# Main execution
LEAK_DETECTED=0

# Try AddressSanitizer first (works on all platforms)
if run_asan; then
    echo "${GREEN}AddressSanitizer: PASS${NC}"
else
    echo "${RED}AddressSanitizer: FAIL${NC}"
    LEAK_DETECTED=1
fi

# Platform-specific tools
case "$PLATFORM" in
    Linux)
        if run_valgrind; then
            echo "${GREEN}Valgrind: PASS${NC}"
        else
            echo "${RED}Valgrind: FAIL${NC}"
            LEAK_DETECTED=1
        fi
        ;;
    Darwin)
        if run_macos_leaks; then
            echo "${GREEN}macOS leaks: PASS${NC}"
        else
            echo "${RED}macOS leaks: FAIL${NC}"
            LEAK_DETECTED=1
        fi
        ;;
    *)
        echo "${YELLOW}No platform-specific memory checker available for $PLATFORM${NC}"
        ;;
esac

echo ""
echo "====================================="
if [ $LEAK_DETECTED -eq 0 ]; then
    echo "${GREEN}All memory checks passed!${NC}"
    exit 0
else
    echo "${RED}Memory issues detected!${NC}"
    echo "Check detailed reports in: $REPORT_DIR"
    exit 1
fi
