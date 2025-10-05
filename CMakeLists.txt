cmake_minimum_required(VERSION 3.16)
project(ssh-client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (try Qt6 first, fallback to Qt5)
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Network)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Find libssh
if(WIN32)
    # On Windows, use vcpkg's find_package
    find_package(libssh CONFIG REQUIRED)
    set(LIBSSH_LIBRARIES ssh)
else()
    # On Unix-like systems, use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBSSH REQUIRED libssh)
    link_directories(${LIBSSH_LIBRARY_DIRS})
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LIBSSH_INCLUDE_DIRS}
)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

# UI files
file(GLOB_RECURSE UI_FILES
    "${CMAKE_SOURCE_DIR}/ui/*.ui"
)

# Main executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# Link libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        ${LIBSSH_LIBRARIES}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        ${LIBSSH_LIBRARIES}
    )
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
    # Link Security framework for Keychain
    find_library(SECURITY_FRAMEWORK Security)
    if(SECURITY_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} ${SECURITY_FRAMEWORK})
    endif()
endif()

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Enable testing (can be disabled with -DENABLE_TESTING=OFF)
option(ENABLE_TESTING "Build tests" ON)
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack configuration for packaging
include(CPack)
set(CPACK_PACKAGE_NAME "SSH-Client")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "SSH Client Project")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()
