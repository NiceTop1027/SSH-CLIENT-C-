# Data Model Specification

**Feature**: SSH Client SpecKit
**Date**: 2025-10-05

## Overview
This document defines the data entities, their attributes, relationships, and validation rules for the SSH client application.

## Core Entities

### 1. ConnectionProfile

**Description**: Represents a saved SSH connection configuration that can be reused.

**Attributes**:
- `id` (UUID): Unique identifier for the profile
- `name` (String): User-defined profile name (e.g., "Production Server", "Dev Box")
- `hostname` (String): Server hostname or IP address
- `port` (Integer): SSH port number (1-65535, default 22)
- `username` (String): SSH login username
- `authMethod` (Enum): Authentication method [PASSWORD, SSH_KEY]
- `sshKeyPath` (String, optional): Path to SSH private key file (required if authMethod = SSH_KEY)
- `folder` (String, optional): Organizational folder/group name
- `autoReconnect` (Boolean): Enable automatic reconnection on disconnect (default false)
- `scrollbackLines` (Integer): Terminal scrollback buffer size (default 10000)
- `createdAt` (Timestamp): Profile creation time
- `lastModified` (Timestamp): Last modification time
- `lastUsed` (Timestamp, optional): Last connection time

**Validation Rules**:
- `name`: Non-empty, max 255 characters, unique across profiles
- `hostname`: Valid DNS name or IPv4/IPv6 address format
- `port`: Integer 1-65535
- `username`: Non-empty, valid Unix username pattern (alphanumeric, underscore, hyphen)
- `sshKeyPath`: Must point to existing readable file when authMethod = SSH_KEY
- `scrollbackLines`: Integer 100-100000

**Relationships**:
- One ConnectionProfile → Many ConnectionSession (historical)
- One ConnectionProfile → Many AuthenticationCredential (password stored separately)

**State Transitions**: N/A (static configuration)

**Storage Format**:
```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "name": "Production Server",
  "hostname": "prod.example.com",
  "port": 22,
  "username": "admin",
  "authMethod": "SSH_KEY",
  "sshKeyPath": "/home/user/.ssh/id_rsa",
  "folder": "Production",
  "autoReconnect": true,
  "scrollbackLines": 10000,
  "createdAt": "2025-10-05T10:30:00Z",
  "lastModified": "2025-10-05T10:30:00Z",
  "lastUsed": "2025-10-05T14:22:00Z"
}
```

---

### 2. ConnectionSession

**Description**: Represents an active or historical SSH connection instance.

**Attributes**:
- `sessionId` (UUID): Unique session identifier
- `profileId` (UUID): Reference to ConnectionProfile
- `status` (Enum): Current connection state [CONNECTING, CONNECTED, DISCONNECTED, ERROR, RECONNECTING]
- `startTime` (Timestamp): When connection was initiated
- `endTime` (Timestamp, optional): When connection was closed
- `connectionDuration` (Integer, computed): Duration in seconds
- `terminalState` (TerminalState): Associated terminal state
- `errorMessage` (String, optional): Last error message if status = ERROR
- `reconnectAttempts` (Integer): Number of reconnection attempts made (default 0)
- `bytesReceived` (Long): Total bytes received from server
- `bytesSent` (Long): Total bytes sent to server

**Validation Rules**:
- `profileId`: Must reference existing ConnectionProfile
- `status`: Must be valid enum value
- `reconnectAttempts`: Non-negative integer

**Relationships**:
- One ConnectionProfile → Many ConnectionSession
- One ConnectionSession → One TerminalBuffer

**State Transitions**:
```
CONNECTING → CONNECTED (on successful auth)
CONNECTING → ERROR (on connection/auth failure)
CONNECTED → DISCONNECTED (user disconnect or normal close)
CONNECTED → RECONNECTING (connection lost, autoReconnect enabled)
CONNECTED → ERROR (connection error)
RECONNECTING → CONNECTED (successful reconnect)
RECONNECTING → ERROR (reconnect failed, max attempts reached)
ERROR → CONNECTING (user retry)
DISCONNECTED → CONNECTING (user reconnect)
```

---

### 3. TerminalBuffer

**Description**: Represents the terminal display state and scrollback history.

**Attributes**:
- `sessionId` (UUID): Reference to ConnectionSession
- `lines` (Array<TerminalLine>): Current visible and scrollback lines
- `cursorRow` (Integer): Current cursor row position (0-based)
- `cursorColumn` (Integer): Current cursor column position (0-based)
- `maxScrollback` (Integer): Maximum number of scrollback lines
- `currentScrollOffset` (Integer): Current scroll position (0 = bottom)
- `dimensions` (TerminalDimensions): Terminal size (rows, columns)
- `textEncoding` (String): Character encoding (default UTF-8)

**Nested Types**:

**TerminalLine**:
- `text` (String): Line content
- `formatting` (Array<TextFormat>): Text formatting/color info
- `lineNumber` (Integer): Line number in buffer

**TextFormat**:
- `start` (Integer): Start column
- `end` (Integer): End column
- `bold` (Boolean): Bold text
- `italic` (Boolean): Italic text
- `underline` (Boolean): Underlined text
- `foregroundColor` (Color): Text color
- `backgroundColor` (Color): Background color

**TerminalDimensions**:
- `rows` (Integer): Terminal height in characters
- `columns` (Integer): Terminal width in characters

**Validation Rules**:
- `cursorRow`: 0 <= cursorRow < dimensions.rows
- `cursorColumn`: 0 <= cursorColumn < dimensions.columns
- `maxScrollback`: Matches ConnectionProfile.scrollbackLines
- `lines.length`: <= maxScrollback

**Relationships**:
- One ConnectionSession → One TerminalBuffer

**Operations**:
- `appendLine(line: String)`: Add new line, evict oldest if at maxScrollback
- `updateCursor(row: Integer, column: Integer)`: Update cursor position
- `clearBuffer()`: Remove all lines
- `scrollTo(offset: Integer)`: Scroll to specific position
- `getVisibleLines()`: Return currently visible lines
- `insertText(text: String)`: Insert text at cursor position
- `applyFormatting(format: TextFormat)`: Apply formatting to current position

---

### 4. AuthenticationCredential

**Description**: Represents stored authentication information for a connection profile.

**Attributes**:
- `profileId` (UUID): Reference to ConnectionProfile
- `credentialType` (Enum): Type of credential [PASSWORD, SSH_KEY_PASSPHRASE]
- `credentialReference` (String): OS keychain reference/key
- `lastUpdated` (Timestamp): When credential was last updated

**Validation Rules**:
- `profileId`: Must reference existing ConnectionProfile
- `credentialType`: Must be valid enum value
- `credentialReference`: Non-empty when credential is stored

**Relationships**:
- One ConnectionProfile → Zero or One AuthenticationCredential

**Security Notes**:
- Actual credential value NEVER stored in application database
- `credentialReference` is an opaque reference to OS credential manager
- Passwords retrieved only when needed for authentication
- Credentials can be optional (user enters each time)

**Storage**:
- NOT stored in JSON config files
- Stored exclusively in OS-native credential managers:
  - Windows: DPAPI / Windows Credential Manager
  - macOS: Keychain Services
  - Linux: libsecret (GNOME Keyring / KWallet)

---

### 5. ApplicationSettings

**Description**: Global application configuration and user preferences.

**Attributes**:
- `theme` (Enum): UI theme [SYSTEM, LIGHT, DARK]
- `defaultPort` (Integer): Default SSH port for new profiles (default 22)
- `defaultScrollback` (Integer): Default scrollback lines (default 10000)
- `defaultAuthMethod` (Enum): Default authentication method [PASSWORD, SSH_KEY]
- `connectionTimeout` (Integer): Connection timeout in seconds (default 30)
- `maxConcurrentConnections` (Integer): Max simultaneous connections (default 50)
- `logLevel` (Enum): Logging verbosity [DEBUG, INFO, WARNING, ERROR]
- `logRotationSize` (Integer): Log file size limit in MB (default 10)
- `logMaxFiles` (Integer): Max log files to retain (default 5)
- `showConnectionNotifications` (Boolean): Show desktop notifications (default true)
- `confirmBeforeDisconnect` (Boolean): Confirm before closing active connections (default true)
- `windowGeometry` (String): Saved window size/position
- `lastProfilesView` (Enum): Last used profiles view [LIST, TREE]

**Validation Rules**:
- `defaultPort`: 1-65535
- `defaultScrollback`: 100-100000
- `connectionTimeout`: 5-300 seconds
- `maxConcurrentConnections`: 1-100
- `logRotationSize`: 1-100 MB
- `logMaxFiles`: 1-20

**Storage Format**: INI or JSON in platform-specific config location
- Windows: `%APPDATA%/SSHClient/settings.ini`
- macOS: `~/Library/Application Support/SSHClient/settings.ini`
- Linux: `~/.config/SSHClient/settings.ini`

---

## Entity Relationships Diagram

```
┌─────────────────────┐
│ ApplicationSettings │  (Global, Singleton)
└─────────────────────┘

┌──────────────────┐       1:N      ┌───────────────────┐
│ ConnectionProfile│◄───────────────┤ ConnectionSession │
└──────────────────┘                └───────────────────┘
        │ 1:0..1                              │ 1:1
        ▼                                     ▼
┌─────────────────────┐            ┌──────────────────┐
│AuthenticationCred   │            │  TerminalBuffer  │
└─────────────────────┘            └──────────────────┘
```

---

## Data Persistence

### Connection Profiles
- **Location**: `~/.ssh-client/profiles/` (or platform equivalent)
- **Format**: One JSON file per profile
- **Filename**: `{profile-id}.json`
- **Access**: Read on app start, write on profile create/update/delete

### Application Settings
- **Location**: Platform-specific config directory
- **Format**: INI or JSON
- **Access**: Read on app start, write on setting change

### Credentials
- **Location**: OS-native credential store
- **Format**: OS-specific encrypted storage
- **Access**: On-demand when authentication needed

### Session State (Runtime Only)
- ConnectionSession and TerminalBuffer are runtime-only
- Not persisted between application runs
- Optionally: Save terminal scrollback to file on user request (future feature)

---

## Data Migration

### Version 1.0 (Initial Release)
- No migration needed

### Future Considerations
- Schema version field in ConnectionProfile
- Migration scripts for backward compatibility
- Import/export functionality for profile portability

---

## Validation Summary

All entities implement validation before persistence:
1. **Type validation**: Ensure correct data types
2. **Range validation**: Check numeric bounds
3. **Format validation**: Verify strings match expected patterns
4. **Referential integrity**: Ensure foreign keys exist
5. **Business rules**: Apply domain-specific constraints

Validation occurs at:
- User input (UI layer)
- Before persistence (model layer)
- After deserialization (when loading from files)

---

**Status**: Data model complete, ready for contract generation
