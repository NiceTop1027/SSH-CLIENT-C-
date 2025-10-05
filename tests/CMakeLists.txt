# Test configuration
cmake_minimum_required(VERSION 3.16)

# Enable Qt Test
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 COMPONENTS Test REQUIRED)
else()
    find_package(Qt5 COMPONENTS Test REQUIRED)
endif()

# Try to find Google Test
find_package(GTest QUIET)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Helper function to create tests
function(add_unit_test test_name)
    add_executable(${test_name} ${ARGN})

    if(QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(${test_name}
            Qt6::Test
            Qt6::Core
            Qt6::Widgets
        )
    else()
        target_link_libraries(${test_name}
            Qt5::Test
            Qt5::Core
            Qt5::Widgets
        )
    endif()

    # Add Google Test if available
    if(GTest_FOUND)
        target_link_libraries(${test_name} GTest::GTest GTest::Main)
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Unit tests directory
add_subdirectory(unit)

# Integration tests directory
add_subdirectory(integration)

# UI tests directory
add_subdirectory(ui)

# Performance tests directory
add_subdirectory(performance)
