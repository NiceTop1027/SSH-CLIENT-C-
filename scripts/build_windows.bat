@echo off
REM Windows build script for SSH Client

echo =======================================
echo SSH Client Windows Build Script
echo =======================================

set PROJECT_NAME=ssh-client
set BUILD_DIR=build_windows
set INSTALL_DIR=dist\windows

REM Check for required tools
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found! Please install CMake.
    exit /b 1
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo.
echo Configuring project...
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX="..\%INSTALL_DIR%"

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

echo.
echo Running tests...
ctest -C Release --output-on-failure

echo.
echo Installing...
cmake --install . --config Release

echo.
echo Creating installer package...
cpack -C Release -G NSIS

cd ..

echo.
echo =======================================
echo Build complete!
echo Binaries: %BUILD_DIR%\Release\
echo Installer: %BUILD_DIR%\packages\
echo =======================================
