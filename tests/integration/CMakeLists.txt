# Integration tests

# Note: These tests will fail until the SSH implementation is complete
# This is intentional - following TDD (Test-Driven Development) approach

add_unit_test(test_password_auth
    test_password_auth.cpp
)

add_unit_test(test_key_auth
    test_key_auth.cpp
)

add_unit_test(test_terminal_display
    test_terminal_display.cpp
)

add_unit_test(test_async_io
    test_async_io.cpp
)

# Disabled due to missing SSHChannel implementation
# add_unit_test(test_e2e_connection
#     test_e2e_connection.cpp
# )
