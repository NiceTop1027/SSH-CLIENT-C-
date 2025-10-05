# SSH Client Architecture

This document describes the architecture and design of the SSH Client application.

## Overview

SSH Client is a cross-platform GUI application built using C++17, Qt, and libssh. It follows an object-oriented, modular design with clear separation of concerns.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                         UI Layer                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  MainWindow  │  │ConnectionDia │  │TerminalWidget│      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
└─────────┼──────────────────┼──────────────────┼─────────────┘
          │                  │                  │
┌─────────┼──────────────────┼──────────────────┼─────────────┐
│         │    Business Logic Layer             │              │
│  ┌──────▼───────┐  ┌──────▼───────┐  ┌──────▼───────┐      │
│  │ProfileStorage│  │SSHAuthenticato│  │  ANSIParser  │      │
│  └──────────────┘  └──────┬───────┘  └──────────────┘      │
│  ┌──────────────┐  ┌──────▼───────┐  ┌──────────────┐      │
│  │CredentialMgr │  │SSHConnection │  │TerminalBuffer│      │
│  └──────────────┘  └──────┬───────┘  └──────────────┘      │
└─────────────────────────────┼───────────────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────┐
│         Threading & I/O Layer│                               │
│  ┌──────────────┐  ┌────────▼─────┐  ┌──────────────┐      │
│  │SSHWorkerThrea│  │  SSHChannel  │  │    Logger    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────┐
│         External Libraries   │                               │
│  ┌──────────────┐  ┌────────▼─────┐  ┌──────────────┐      │
│  │   Qt (GUI)   │  │    libssh    │  │Platform APIs │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────────────────────────────────────────────┘
```

## Component Description

### UI Layer

#### MainWindow
- **Purpose**: Main application window
- **Responsibilities**:
  - Manage multiple terminal tabs
  - Handle user actions (menu, shortcuts)
  - Coordinate between UI components
  - Display status messages
- **Key Features**:
  - QTabWidget for multiple sessions
  - Menu bar with File/Edit/Help menus
  - Status bar for connection status

#### ConnectionDialog
- **Purpose**: Connection configuration dialog
- **Responsibilities**:
  - Collect connection parameters
  - Validate user input
  - Support password and key authentication
  - Save/load connection profiles
- **Key Features**:
  - Dynamic UI based on auth method
  - File browser for key selection
  - Input validation

#### TerminalWidget
- **Purpose**: Interactive terminal display
- **Responsibilities**:
  - Display terminal output
  - Capture user input
  - Handle keyboard events
  - Support text selection and copy/paste
- **Key Features**:
  - Based on QTextEdit
  - ANSI color and formatting support
  - Scrollback buffer
  - Special key handling (arrows, Ctrl+C, etc.)

### Business Logic Layer

#### ConnectionProfile
- **Purpose**: Data model for connection configuration
- **Responsibilities**:
  - Store connection parameters (host, port, user)
  - Store authentication settings
  - Serialize to/from JSON
- **Key Features**:
  - Immutable after creation (mostly)
  - JSON serialization
  - Validation methods

#### SSHConnection
- **Purpose**: SSH protocol layer using libssh
- **Responsibilities**:
  - Establish SSH connections
  - Manage session lifecycle
  - Emit signals for connection state changes
  - Error handling and categorization
- **Key Features**:
  - Asynchronous connection setup
  - Host key verification
  - Multiple error types
  - Thread-safe
- **State Machine**:
  ```
  Disconnected → Connecting → Connected
       ↑              ↓           ↓
       └───────── Error ──────────┘
  ```

#### SSHAuthenticator
- **Purpose**: Authentication handling
- **Responsibilities**:
  - Support password authentication
  - Support public key authentication
  - Handle authentication retries
- **Key Features**:
  - Pluggable design
  - Secure credential handling
  - Passphrase support for keys

#### SSHChannel
- **Purpose**: SSH channel operations
- **Responsibilities**:
  - Open shell/exec channels
  - Request PTY
  - Read/write data
  - Handle channel lifecycle
- **Key Features**:
  - Non-blocking I/O
  - Buffer management
  - PTY size control

#### ProfileStorage
- **Purpose**: Persist connection profiles
- **Responsibilities**:
  - Save profiles to disk
  - Load profiles from disk
  - Manage storage location
- **Key Features**:
  - JSON file format
  - Per-profile files
  - Platform-specific storage paths

#### CredentialManager
- **Purpose**: Secure credential storage
- **Responsibilities**:
  - Store passwords securely
  - Retrieve stored credentials
  - Platform-specific implementations
- **Platform Support**:
  - macOS: Keychain
  - Windows: DPAPI
  - Linux: libsecret (or encrypted file fallback)

#### TerminalBuffer
- **Purpose**: Terminal text buffer management
- **Responsibilities**:
  - Store terminal lines
  - Manage scrollback
  - Support text selection
- **Key Features**:
  - Efficient scrollback (ring buffer)
  - Configurable max lines (default: 10,000)
  - Line wrapping

#### ANSIParser
- **Purpose**: Parse ANSI escape sequences
- **Responsibilities**:
  - Parse VT100/ANSI sequences
  - Convert to formatting tokens
  - Handle cursor control, colors, text attributes
- **Supported Sequences**:
  - Cursor movement (CUU, CUD, CUF, CUB, CUP)
  - Erase functions (ED, EL)
  - Graphics (SGR) - colors, bold, underline, etc.
  - Basic VT100 compatibility

### Threading & I/O Layer

#### SSHWorkerThread
- **Purpose**: Background I/O thread for SSH
- **Responsibilities**:
  - Read data from SSH channel asynchronously
  - Write data to SSH channel
  - Emit signals for received data
  - Handle disconnection
- **Key Features**:
  - QThread-based
  - Thread-safe write queue
  - Non-blocking read loop
  - Graceful shutdown

#### Logger
- **Purpose**: Application-wide logging
- **Responsibilities**:
  - Log to file and console
  - Rotate log files
  - Categorized logging
- **Key Features**:
  - Qt logging categories (ssh.connection, ssh.auth, terminal, ui, storage)
  - File rotation (max 10MB, 5 files)
  - ISO timestamp format
  - Thread-safe

#### ErrorDialog
- **Purpose**: User-friendly error reporting
- **Responsibilities**:
  - Display categorized error messages
  - Log errors automatically
  - Provide context-specific error information
- **Error Types**:
  - Connection errors
  - Authentication errors
  - Network errors
  - File operation errors
  - Configuration errors

## Design Patterns

### Observer Pattern (Signals & Slots)
```cpp
// SSHConnection emits signals
connect(connection, &SSHConnection::connected,
        this, &MainWindow::onConnected);
connect(connection, &SSHConnection::error,
        this, &MainWindow::onError);
```

### Singleton Pattern
```cpp
// Logger singleton
Logger& Logger::instance() {
    static Logger instance;
    return instance;
}
```

### Strategy Pattern
```cpp
// Authentication strategies
class SSHAuthenticator {
    bool authenticate(ssh_session session);
protected:
    virtual bool doAuthenticate(ssh_session session) = 0;
};
```

### Factory Pattern
```cpp
// Profile creation from JSON
ConnectionProfile profile = ConnectionProfile::fromJson(json);
```

### RAII (Resource Acquisition Is Initialization)
```cpp
// Automatic cleanup
~SSHConnection() {
    if (m_session) {
        ssh_disconnect(m_session);
        ssh_free(m_session);
    }
}
```

## Threading Model

### Main Thread
- UI operations (Qt requirement)
- User input handling
- Profile management

### Worker Threads (SSHWorkerThread)
- SSH I/O operations
- One thread per connection
- Communication via signals/slots

### Thread Safety
- QMutex for shared data (write queue)
- Qt::QueuedConnection for cross-thread signals
- No shared mutable state between threads

## Data Flow

### Connection Establishment
```
User Input (ConnectionDialog)
    ↓
MainWindow::handleConnectionRequest
    ↓
SSHConnection::connectToHost
    ↓
SSHAuthenticator::authenticate
    ↓
SSHConnection::connected (signal)
    ↓
MainWindow::handleConnected
    ↓
SSHWorkerThread::start
    ↓
Ready for I/O
```

### Data Transfer
```
User Keypress (TerminalWidget)
    ↓
TerminalWidget::keyPressEvent
    ↓
TerminalWidget::sendData (signal)
    ↓
SSHWorkerThread::writeData (queued)
    ↓
SSHChannel::writeBytes (worker thread)
    ↓
libssh → Remote Server

Remote Server → libssh
    ↓
SSHChannel::readBytes (worker thread)
    ↓
SSHWorkerThread::dataReceived (signal)
    ↓
MainWindow::handleDataReceived
    ↓
TerminalWidget::displayOutput
    ↓
ANSIParser::parseToTokens
    ↓
TerminalWidget renders formatted text
```

## Error Handling

### Error Propagation
1. **Low-level (libssh)**:
   - Return codes checked
   - ssh_get_error() for messages

2. **Mid-level (SSHConnection)**:
   - Categorize errors (ErrorType enum)
   - Log with appropriate severity
   - Emit error signals

3. **High-level (UI)**:
   - Display user-friendly messages
   - Offer recovery actions
   - Maintain application state

### Error Recovery
- Automatic reconnection (planned)
- Graceful degradation
- Clear error messages
- Cleanup on error

## Security Considerations

### Credentials
- Never log passwords
- Use secure platform storage
- Clear sensitive data from memory

### Host Key Verification
- Current: Accept all (insecure - for development)
- TODO: Implement known_hosts checking
- TODO: Prompt user for unknown hosts

### Data Transmission
- All data over SSH (encrypted)
- No plaintext credential storage
- Secure memory handling

## Performance Optimizations

### Memory Management
- Ring buffer for scrollback (bounded memory)
- Lazy loading of profiles
- Qt parent-child ownership (automatic cleanup)

### I/O Optimization
- Buffered writes (write queue)
- Non-blocking reads
- Separate thread for I/O

### UI Optimization
- Batch terminal updates
- Efficient ANSI parsing
- Double-buffered rendering (Qt default)

## Testing Strategy

### Unit Tests
- Test individual classes in isolation
- Mock dependencies
- Focus on business logic

### Integration Tests
- Test component interactions
- Use real SSH connections (test server)
- Environment-based configuration

### UI Tests
- Simulate user interactions
- Test dialog workflows
- Verify UI state

### Performance Tests
- Benchmark critical paths
- Monitor memory usage
- Profile slow operations

## Build System

### CMake Structure
```
CMakeLists.txt (root)
    ├── Qt and libssh detection
    ├── Compiler flags
    ├── Source file globbing
    └── tests/CMakeLists.txt
            ├── unit/CMakeLists.txt
            ├── integration/CMakeLists.txt
            ├── ui/CMakeLists.txt
            └── performance/CMakeLists.txt
```

### Cross-Platform Support
- Qt handles most platform differences
- Platform-specific code isolated (#ifdef guards)
- Separate build scripts per platform

## Future Enhancements

### Planned Features
- SFTP support
- Port forwarding (local/remote)
- X11 forwarding
- Session save/restore
- Custom key bindings
- Themes and customization

### Architecture Extensions
- Plugin system for protocols
- Scripting support (Lua/JavaScript)
- Cloud sync for profiles
- Multi-factor authentication

## Dependencies

### Required
- **Qt**: GUI framework (5.15+ or 6.x)
- **libssh**: SSH protocol (0.9.0+)
- **CMake**: Build system (3.16+)

### Optional
- **GTest**: Additional testing framework
- **Valgrind**: Memory leak detection (Linux)
- **ASAN**: Address sanitizer (all platforms)

## Maintenance

### Code Quality
- Run clang-format before commits
- Keep test coverage >80%
- Regular dependency updates
- Security audits

### Documentation
- Update architecture docs with major changes
- Document public APIs
- Keep user guide current
- Maintain changelog

---

**Last Updated**: 2025-10-05
**Version**: 1.0.0
