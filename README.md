# SSH Client

A modern, cross-platform GUI SSH client built with C++ and Qt.

## 🚀 Quick Download

Download the latest release for your platform:

- **macOS**: [ssh-client-macos.dmg](https://github.com/YOUR_USERNAME/ssh-program/releases/latest/download/ssh-client-macos.dmg)
- **Windows**: [ssh-client-windows.zip](https://github.com/YOUR_USERNAME/ssh-program/releases/latest/download/ssh-client-windows.zip)
- **Linux**: [ssh-client-linux.AppImage](https://github.com/YOUR_USERNAME/ssh-program/releases/latest/download/ssh-client-linux.AppImage)

## ✨ Features

- **Full Terminal Emulation**: Complete VT100/xterm-256color support with vi/vim compatibility
- **SSH Connection Management**: Password and SSH key authentication
- **File Type Colorization**: Visual Studio Code-like syntax highlighting for file listings
- **Session Management**: Save and organize connection profiles
- **Cross-Platform**: Native support for Windows, macOS, and Linux
- **Multiple Sessions**: Tabbed interface for multiple SSH connections
- **Alternate Screen Buffer**: Full-screen applications (vi/vim/htop/top) work perfectly
- **256 Color Support**: Rich terminal colors and themes

## Requirements

### Build Dependencies

- **C++ Compiler**: C++17 or later (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake**: 3.16 or later
- **Qt**: Qt5 (5.15+) or Qt6
- **libssh**: SSH protocol library
- **pkg-config**: For finding libssh

### Platform-Specific Notes

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install build-essential cmake qt6-base-dev libssh-dev pkg-config
```

#### Linux (Fedora)
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel libssh-devel pkgconfig
```

#### macOS
```bash
brew install cmake qt libssh pkg-config
```

#### Windows
- Install [Visual Studio 2019+](https://visualstudio.microsoft.com/) or [MinGW-w64](https://www.mingw-w64.org/)
- Install [CMake](https://cmake.org/download/)
- Install [Qt](https://www.qt.io/download) (use online installer)
- Install [libssh](https://www.libssh.org/) (via vcpkg or manual build)

## Building

### Quick Start with Platform Scripts

The easiest way to build is using the provided platform-specific scripts:

```bash
# Linux
./scripts/build_linux.sh

# macOS
./scripts/build_macos.sh

# Windows
scripts\build_windows.bat
```

### Standard Build

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests
ctest

# Install (optional)
sudo cmake --install .
```

### Build with Specific Qt Version

```bash
# For Qt6
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 ..

# For Qt5
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15/gcc_64 ..
```

### Build Types

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Running

After building, the executable will be in the `build` directory:

```bash
# Linux/macOS
./ssh-client

# Windows
ssh-client.exe
```

## Project Structure

```
ssh-program/
├── src/            # Source files (.cpp)
├── include/        # Header files (.h)
├── tests/          # Unit and integration tests
├── resources/      # Icons, images, and other resources
├── docs/           # Documentation
├── ui/             # Qt Designer UI files
└── specs/          # Feature specifications and design documents
```

## Testing

The project uses Qt Test framework with comprehensive test coverage:

```bash
# Run all tests
cd build
ctest

# Run tests with verbose output
ctest --verbose

# Run specific test
./tests/unit/test_connection_profile

# Run integration tests (requires SSH test server)
export SSH_TEST_HOST=localhost
export SSH_TEST_USER=testuser
export SSH_TEST_PASSWORD=testpass
./tests/integration/test_e2e_connection

# Run performance benchmarks
./tests/performance/test_performance

# Check for memory leaks
../scripts/check_memory_leaks.sh
```

## Documentation

Detailed documentation is available in the `docs/` directory:

- [Building Guide](docs/BUILDING.md) - Detailed build instructions
- [User Guide](docs/USER_GUIDE.md) - How to use the application
- [Contributing](docs/CONTRIBUTING.md) - Development guidelines

## License

[To be determined]

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## Support

For issues and questions:
- Open an issue on GitHub
- Check existing documentation in the `docs/` directory
