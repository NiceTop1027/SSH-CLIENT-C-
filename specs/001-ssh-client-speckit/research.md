# Phase 0: Research & Technical Decisions

**Feature**: SSH Client SpecKit
**Date**: 2025-10-05

## Overview
This document consolidates research findings and technical decisions for the SSH client application. It addresses clarifications needed from the specification and establishes best practices for the chosen technology stack.

## Technical Stack Decisions

### 1. Terminal Emulation Level

**Decision**: VT100/ANSI escape sequence support with xterm extensions

**Rationale**:
- VT100 is the baseline standard supported by virtually all SSH servers
- ANSI escape sequences handle colors, cursor positioning, and screen clearing
- Xterm extensions provide commonly-used features (256 colors, mouse events)
- Qt's QTextEdit can be extended to handle these sequences programmatically

**Alternatives Considered**:
- Full xterm compatibility: Too complex for initial release, diminishing returns
- Plain text terminal: Insufficient - many server applications require escape sequence support
- VT220: More features but marginal benefit over VT100+ANSI for typical use cases

**Implementation Notes**:
- Parse and interpret ANSI escape sequences for text formatting, colors, cursor control
- Maintain cursor position state
- Support scrollback buffer
- Defer advanced features (mouse reporting, alternate screen buffer) to future releases

### 2. Credential Storage Security

**Decision**: Store connection profiles with optional password storage using OS-native credential managers

**Rationale**:
- Windows: DPAPI (Data Protection API) for encrypted storage
- macOS: Keychain Services API
- Linux: libsecret (GNOME Keyring / KWallet integration)
- Provides OS-integrated security without reinventing encryption
- User expectations align with OS-native password management

**Alternatives Considered**:
- Plain text storage: Unacceptable security risk
- Application-level encryption: Requires secure key management, more complex
- No password storage: Forces re-entry every time, poor UX

**Implementation Notes**:
- Connection profiles stored in JSON/XML config files with metadata only
- Passwords stored separately in OS credential manager, referenced by profile ID
- SSH key paths stored in config (keys themselves remain in user's .ssh directory)
- Provide user option to not save passwords (enter each time)

### 3. Concurrent Connection Support

**Decision**: Support multiple concurrent SSH connections via tabbed interface

**Rationale**:
- System administrators frequently need to work with multiple servers simultaneously
- Tabbed interface is familiar UX pattern (browsers, terminal emulators)
- Each connection operates independently with separate terminal state
- Resource limits: Max 50 concurrent connections (configurable)

**Alternatives Considered**:
- Single connection only: Too limiting for target user base
- Multi-window approach: More complex window management, less convenient
- Unlimited connections: Risk of resource exhaustion

**Implementation Notes**:
- QTabWidget for tab management
- Each tab contains independent TerminalWidget instance
- Separate SSH session and channel per connection
- Thread per connection for non-blocking I/O
- Connection limit configurable via settings

### 4. Session Profile Organization

**Decision**: Flat list with search/filter and optional folder grouping

**Rationale**:
- Start simple: Alphabetically sorted flat list
- Search/filter handles moderate numbers of profiles (100-500)
- Optional folder grouping for users with many servers
- Tags can be added in future releases if needed

**Alternatives Considered**:
- Folder hierarchy only: Requires upfront organization, rigid
- Tags only: More flexible but requires more UI complexity
- Flat list only: Becomes unwieldy beyond 50-100 profiles

**Implementation Notes**:
- Profiles stored in `~/.ssh-client/profiles/` directory (or platform-specific config location)
- Each profile is separate JSON file for easy management
- Optional `folder` field in profile metadata
- UI provides tree view (folders) and list view (flat) options

### 5. Error Handling and Logging

**Decision**: Structured logging with Qt logging framework, user-facing error dialogs with details

**Rationale**:
- Qt provides built-in logging categories and severity levels
- Logs written to platform-specific locations (XDG_DATA_HOME, AppData, etc.)
- Log rotation to prevent unbounded growth (10MB max per file, 5 files)
- User error messages separate from debug logs

**Alternatives Considered**:
- Console output only: Not accessible for deployed desktop app
- External logging library (spdlog): Additional dependency, Qt's framework sufficient
- No logging: Debugging and support impossible

**Implementation Notes**:
- Log categories: ssh.connection, ssh.auth, terminal, ui, storage
- Severity levels: debug, info, warning, critical
- User error messages: Connection failures, authentication errors, file I/O errors
- Technical details in logs, user-friendly messages in dialogs

### 6. Automatic Reconnection Strategy

**Decision**: Optional automatic reconnection with exponential backoff

**Rationale**:
- Many disconnections are transient (network blips, server restarts)
- Exponential backoff prevents hammering unreachable servers
- User should be able to disable for manual control

**Alternatives Considered**:
- Always reconnect: Annoying for intentional disconnections
- Never reconnect: Forces manual intervention for temporary issues
- Fixed interval retries: Can overload server or network during extended outages

**Implementation Notes**:
- Enabled per-profile (default: disabled)
- Retry intervals: 2s, 5s, 10s, 30s, 60s (max 5 attempts)
- User notification of reconnection attempts
- Cancel button to stop automatic reconnection
- Reset backoff on successful connection

### 7. Terminal Buffer Management

**Decision**: Configurable scrollback buffer with 10,000 line default, LRU eviction

**Rationale**:
- 10k lines balances memory usage (≈1-2MB per terminal) with utility
- Users can adjust based on their needs (1k-100k range)
- LRU eviction maintains recent history during long sessions
- QTextEdit handles rendering efficiently

**Alternatives Considered**:
- Unlimited buffer: Risk of memory exhaustion in long sessions
- Fixed 1000 lines: Too limited for debugging or log review
- Disk-backed buffer: Complexity not justified for initial release

**Implementation Notes**:
- Setting stored per-profile or globally
- Buffer size configurable in preferences
- Warning if setting above 50k lines
- Clear buffer action in UI

### 8. Connection Validation

**Decision**: Client-side validation before connection attempt

**Rationale**:
- Prevent obviously invalid connection attempts
- Provide immediate feedback rather than server timeout
- Reduce error cases to handle

**Validation Rules**:
- Hostname/IP: Valid DNS name or IP address format
- Port: Integer in range 1-65535
- Username: Non-empty, valid Unix username characters
- SSH key: File exists and is readable (if key auth selected)

**Alternatives Considered**:
- No validation: Poor UX, delayed error feedback
- Server-side validation only: Not applicable (external SSH servers)

### 9. Platform-Specific Considerations

**Decision**: Platform-specific builds with native look-and-feel

**Rationale**:
- Qt provides native theming on each platform
- Users expect platform conventions (menus, shortcuts, dialogs)
- Single codebase with platform-specific packaging

**Platform Requirements**:
- **Windows 10+**: MSVC or MinGW compiler, Windows SDK, NSIS installer
- **macOS 10.15+**: Xcode, app bundle, code signing for distribution
- **Linux**: GCC/Clang, .deb and .rpm packages, AppImage for universal support

**Implementation Notes**:
- Use Qt's platform abstractions (QStandardPaths, QSettings)
- Platform-specific code isolated with `#ifdef` guards
- CI/CD builds for all platforms
- Testing required on each target platform

### 10. UI Responsiveness Strategy

**Decision**: Dedicated network I/O thread per connection, Qt signals/slots for UI updates

**Rationale**:
- SSH operations (connect, read, write) are blocking
- UI must remain responsive during network operations
- Qt's signal/slot mechanism provides thread-safe UI updates
- QThread per connection isolates failures

**Alternatives Considered**:
- Async I/O with libssh callbacks: More complex, error-prone
- Single worker thread pool: Connection failures could affect others
- All operations on UI thread: UI freezes unacceptable

**Implementation Notes**:
- SSHConnectionWorker class inheriting QThread
- Emit signals for: connection status, data received, errors
- UI thread updates terminal display via slots
- Connection timeout: 30 seconds default, configurable

## Dependencies and Versions

### Core Libraries
- **Qt**: 5.15 LTS or 6.5+ (target both for compatibility)
- **libssh**: 0.10.0+ (latest stable with security fixes)
- **CMake**: 3.16+ (modern CMake practices)

### Testing Libraries
- **Qt Test**: Included with Qt, for UI and integration tests
- **Google Test**: 1.12+ for non-Qt unit tests
- **Google Mock**: For mocking SSH server interactions in tests

### Build and Packaging
- **CMake**: Build system
- **CPack**: Package generation (cross-platform)
- **Qt Installer Framework**: Optional for rich installer experience

## Best Practices

### Memory Management
- Use Qt's parent-child ownership for QObjects (automatic deletion)
- `std::unique_ptr` and `std::shared_ptr` for non-QObject types
- RAII pattern for resource management (connections, files, SSH sessions)
- Regular testing with Valgrind (Linux) and Address Sanitizer

### Error Handling
- Exceptions for unexpected errors (programming bugs)
- Return codes/Result types for expected errors (connection failures)
- User-facing errors presented in dialogs with actionable messages
- All errors logged with context

### Code Organization
- Header-only interface definitions (`include/`)
- Implementation in `src/` with matching directory structure
- One class per file (Qt convention)
- MOC-generated files in build directory

### Testing Strategy
- Unit tests for models, SSH logic, terminal emulation
- Integration tests for end-to-end workflows
- Mock SSH server for testing without real servers
- UI tests using Qt Test framework's QTest::mouseClick, keyClicks

### Security
- Never log passwords or private keys
- Validate all user input
- Use libssh's host key verification
- Secure credential storage (OS keychains)
- Clear sensitive data from memory when done

## Open Questions for Future Consideration
- Internationalization/localization strategy
- Accessibility features (screen reader support)
- Plugin architecture for extensions
- SFTP file transfer integration
- Session recording/playback
- Scripting/automation support

## References
- [Qt Documentation](https://doc.qt.io/)
- [libssh Tutorial](https://api.libssh.org/stable/libssh_tutorial.html)
- [VT100 Specification](https://vt100.net/docs/)
- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)

---

**Status**: Research complete, ready for Phase 1 design
