# Interface Contract: IConnectionManager

**Component**: SSH Connection Management
**Version**: 1.0
**Date**: 2025-10-05

## Purpose
Manages SSH connections lifecycle including connection establishment, authentication, disconnection, and reconnection logic.

## Interface Definition

```cpp
class IConnectionManager {
public:
    virtual ~IConnectionManager() = default;

    // Connection lifecycle
    virtual Result<SessionId> connect(const ConnectionProfile& profile) = 0;
    virtual Result<void> disconnect(SessionId sessionId) = 0;
    virtual ConnectionStatus getStatus(SessionId sessionId) const = 0;

    // Authentication
    virtual Result<void> authenticatePassword(SessionId sessionId, const std::string& password) = 0;
    virtual Result<void> authenticateKey(SessionId sessionId, const std::string& keyPath,
                                         const std::string& passphrase = "") = 0;

    // Data transmission
    virtual Result<void> sendData(SessionId sessionId, const std::string& data) = 0;
    virtual Result<std::string> receiveData(SessionId sessionId) = 0;

    // Connection management
    virtual std::vector<SessionId> getActiveSessions() const = 0;
    virtual Result<void> enableAutoReconnect(SessionId sessionId, bool enable) = 0;

    // Signals (Qt-style)
    // void connected(SessionId sessionId);
    // void disconnected(SessionId sessionId);
    // void dataReceived(SessionId sessionId, const std::string& data);
    // void error(SessionId sessionId, const std::string& errorMessage);
    // void reconnecting(SessionId sessionId, int attemptNumber);
};
```

## Types

### SessionId
```cpp
using SessionId = QUuid;
```

### ConnectionStatus
```cpp
enum class ConnectionStatus {
    Connecting,
    Connected,
    Disconnected,
    Error,
    Reconnecting
};
```

### Result<T>
```cpp
template<typename T>
class Result {
public:
    bool isSuccess() const;
    bool isError() const;
    T value() const;  // Only valid if isSuccess()
    std::string error() const;  // Only valid if isError()
};
```

## Method Contracts

### connect(profile)
**Purpose**: Initiates SSH connection to remote server

**Preconditions**:
- `profile` must be valid (passes ConnectionProfile validation)
- Number of active connections < maxConcurrentConnections

**Postconditions** (success):
- Returns `Result<SessionId>` with new session ID
- Connection status = Connecting
- `connected` signal emitted on successful connection
- `error` signal emitted on failure

**Errors**:
- `MAX_CONNECTIONS_REACHED`: Too many concurrent connections
- `INVALID_PROFILE`: Profile validation failed
- `CONNECTION_FAILED`: Network error or server unreachable
- `TIMEOUT`: Connection timed out

---

### disconnect(sessionId)
**Purpose**: Closes SSH connection and cleans up resources

**Preconditions**:
- `sessionId` must exist in active sessions

**Postconditions** (success):
- Connection status = Disconnected
- Session removed from active sessions
- `disconnected` signal emitted
- SSH resources freed

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID

---

### authenticatePassword(sessionId, password)
**Purpose**: Authenticates to SSH server using password

**Preconditions**:
- `sessionId` must exist
- Connection status = Connecting
- Profile auth method = PASSWORD

**Postconditions** (success):
- Connection status = Connected
- `connected` signal emitted

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID
- `AUTH_FAILED`: Incorrect password or authentication rejected
- `WRONG_AUTH_METHOD`: Profile configured for different auth method
- `ALREADY_AUTHENTICATED`: Session already authenticated

---

### authenticateKey(sessionId, keyPath, passphrase)
**Purpose**: Authenticates to SSH server using SSH private key

**Preconditions**:
- `sessionId` must exist
- Connection status = Connecting
- Profile auth method = SSH_KEY
- Key file at `keyPath` exists and is readable

**Postconditions** (success):
- Connection status = Connected
- `connected` signal emitted

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID
- `KEY_FILE_NOT_FOUND`: Key file doesn't exist
- `KEY_LOAD_FAILED`: Failed to parse key file
- `PASSPHRASE_REQUIRED`: Key is encrypted but no passphrase provided
- `PASSPHRASE_INCORRECT`: Wrong passphrase for encrypted key
- `AUTH_FAILED`: Server rejected key authentication
- `WRONG_AUTH_METHOD`: Profile configured for different auth method

---

### sendData(sessionId, data)
**Purpose**: Sends data to remote server terminal

**Preconditions**:
- `sessionId` must exist
- Connection status = Connected

**Postconditions** (success):
- Data transmitted to server
- Bytes sent counter incremented

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID
- `NOT_CONNECTED`: Session not in connected state
- `SEND_FAILED`: Network error during transmission

---

### receiveData(sessionId)
**Purpose**: Retrieves data received from remote server

**Preconditions**:
- `sessionId` must exist
- Connection status = Connected

**Postconditions** (success):
- Returns buffered data from server
- Buffer cleared after retrieval
- Bytes received counter incremented

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID
- `NOT_CONNECTED`: Session not in connected state
- `RECEIVE_FAILED`: Network error during reception

**Note**: Prefer using `dataReceived` signal for asynchronous notification

---

### getActiveSessions()
**Purpose**: Returns list of all active session IDs

**Preconditions**: None

**Postconditions**:
- Returns vector of SessionIds for all active connections
- Empty vector if no active sessions

**Errors**: None (no-fail guarantee)

---

### enableAutoReconnect(sessionId, enable)
**Purpose**: Enable or disable automatic reconnection for session

**Preconditions**:
- `sessionId` must exist

**Postconditions** (success):
- Auto-reconnect enabled/disabled for session
- If enabled and disconnection occurs, reconnection attempts begin

**Errors**:
- `SESSION_NOT_FOUND`: Invalid session ID

---

## Signals

### connected(sessionId)
**Emitted when**: SSH connection successfully authenticated and ready

**Payload**: SessionId of connected session

---

### disconnected(sessionId)
**Emitted when**: SSH connection closed (user action or server disconnect)

**Payload**: SessionId of disconnected session

---

### dataReceived(sessionId, data)
**Emitted when**: Data received from remote server

**Payload**:
- SessionId: Which session received data
- data: String containing received data

---

### error(sessionId, errorMessage)
**Emitted when**: Error occurs during connection or communication

**Payload**:
- SessionId: Session that encountered error
- errorMessage: Human-readable error description

---

### reconnecting(sessionId, attemptNumber)
**Emitted when**: Automatic reconnection attempt begins

**Payload**:
- SessionId: Session being reconnected
- attemptNumber: Which retry attempt (1-5)

---

## Thread Safety

- All methods must be thread-safe
- Signals emitted on UI thread using Qt's queued connections
- Internal SSH operations execute on dedicated worker threads
- Session state protected by mutex

---

## Performance Requirements

- `connect()`: Complete within connectionTimeout (default 30s)
- `disconnect()`: Complete within 5s
- `sendData()`: Non-blocking, buffered writes
- `receiveData()`: Return immediately with available data
- Signal emission: < 10ms from event occurrence

---

## Test Scenarios

1. **Successful password connection**: connect() → authenticatePassword() → connected signal
2. **Successful key connection**: connect() → authenticateKey() → connected signal
3. **Connection failure**: connect() → error signal, status = Error
4. **Authentication failure**: connect() → authenticatePassword(wrong) → error signal
5. **Disconnect**: disconnect() → disconnected signal, session removed
6. **Data transmission**: sendData() + receiveData() roundtrip
7. **Auto-reconnect**: disconnect (simulated network issue) → reconnecting signal → connected signal
8. **Max connections**: connect() when at limit → MAX_CONNECTIONS_REACHED error
9. **Concurrent sessions**: Multiple connect() → all succeed, independent operation

---

**Status**: Contract defined, ready for test generation
