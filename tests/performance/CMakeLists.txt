# Performance tests

add_unit_test(test_performance
    test_performance.cpp
)

# Add benchmark flag to performance tests
target_compile_definitions(test_performance PRIVATE
    QT_NO_DEBUG_OUTPUT
)

# Optional: Add performance-specific compiler optimizations
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    if(MSVC)
        target_compile_options(test_performance PRIVATE /O2)
    else()
        target_compile_options(test_performance PRIVATE -O2)
    endif()
endif()
