# UI tests

# Note: These tests will fail until the UI implementation is complete
# This is intentional - following TDD (Test-Driven Development) approach

add_unit_test(test_main_window
    test_main_window.cpp
)

add_unit_test(test_connection_dialog
    test_connection_dialog.cpp
)
