# Unit tests

# Note: These tests will fail until the implementation classes are created
# This is intentional - following TDD (Test-Driven Development) approach

add_unit_test(test_connection_profile
    test_connection_profile.cpp
)

add_unit_test(test_session_state
    test_session_state.cpp
)

add_unit_test(test_terminal_buffer
    test_terminal_buffer.cpp
)

add_unit_test(test_ssh_connection
    test_ssh_connection.cpp
)

add_unit_test(test_ssh_authenticator
    test_ssh_authenticator.cpp
)

add_unit_test(test_ansi_parser
    test_ansi_parser.cpp
)

add_unit_test(test_terminal_input
    test_terminal_input.cpp
)

add_unit_test(test_ssh_worker
    test_ssh_worker.cpp
)

add_unit_test(test_profile_storage
    test_profile_storage.cpp
)

add_unit_test(test_credential_manager
    test_credential_manager.cpp
)
