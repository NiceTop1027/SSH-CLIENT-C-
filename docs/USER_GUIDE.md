# SSH Client User Guide

Welcome to the SSH Client User Guide. This document will help you get started and make the most of the application.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Creating Connections](#creating-connections)
3. [Authentication Methods](#authentication-methods)
4. [Using the Terminal](#using-the-terminal)
5. [Managing Connection Profiles](#managing-connection-profiles)
6. [Keyboard Shortcuts](#keyboard-shortcuts)
7. [Troubleshooting](#troubleshooting)
8. [Advanced Features](#advanced-features)

## Getting Started

### First Launch

When you first launch SSH Client, you'll see the main window with an empty tab area and a menu bar.

To create your first connection:

1. Click **File → New Connection** (or press Ctrl+N / Cmd+N)
2. Fill in the connection details
3. Click **Connect**

### Main Window Overview

The main window consists of:

- **Menu Bar**: Access all application features
- **Tab Bar**: Manage multiple concurrent SSH sessions
- **Terminal Area**: Interactive terminal for each connection
- **Status Bar**: Display connection status and messages

## Creating Connections

### Quick Connection

For a one-time connection:

1. Go to **File → New Connection**
2. Enter the required information:
   - **Hostname**: The server's IP address or domain name (e.g., `example.com` or `192.168.1.100`)
   - **Port**: SSH port (default: 22)
   - **Username**: Your username on the remote server
3. Choose authentication method (Password or Public Key)
4. Click **Connect**

### Creating a Named Profile

To save a connection for future use:

1. Go to **File → New Connection**
2. Enter a **Profile Name** (e.g., "Production Server", "Development VM")
3. Fill in connection details
4. Click **Connect**

The profile will be automatically saved and available in **File → Recent Connections**.

## Authentication Methods

### Password Authentication

1. In the Connection Dialog, select **Password** from the Authentication Method dropdown
2. You'll be prompted for your password when connecting
3. The password can optionally be saved securely in your system keychain

**Security Note**: Passwords are stored using platform-specific secure storage:
- **macOS**: Keychain
- **Windows**: DPAPI (Data Protection API)
- **Linux**: libsecret (or encrypted file as fallback)

### Public Key Authentication

1. In the Connection Dialog, select **Public Key** from the Authentication Method dropdown
2. Click **Browse** to select your private key file (typically in `~/.ssh/`)
3. Common key files:
   - `id_rsa` - RSA key
   - `id_ed25519` - Ed25519 key (recommended)
   - `id_ecdsa` - ECDSA key

**Key Requirements**:
- Your private key must be readable (permissions 600 on Unix systems)
- If your key has a passphrase, you'll be prompted to enter it
- The corresponding public key must be in the server's `~/.ssh/authorized_keys`

### Generating SSH Keys

If you don't have an SSH key pair:

```bash
# Generate an Ed25519 key (recommended)
ssh-keygen -t ed25519 -C "your_email@example.com"

# Or generate an RSA key
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
```

Then copy your public key to the server:

```bash
ssh-copy-id username@hostname
```

## Using the Terminal

### Basic Usage

Once connected, you can:
- Type commands directly in the terminal
- Use standard shell features (command history, tab completion, etc.)
- Copy text by selecting it
- Paste text using Ctrl+V (Cmd+V on macOS) or **Edit → Paste**

### Terminal Features

- **ANSI Color Support**: Full 256-color support
- **Scrollback Buffer**: Keep up to 10,000 lines of history
- **Text Selection**: Click and drag to select text
- **Find in Buffer**: Ctrl+F to search terminal output

### Special Keys

The terminal supports special key sequences:
- **Arrow Keys**: Navigate command history and move cursor
- **Home/End**: Jump to beginning/end of line
- **Page Up/Down**: Scroll through output
- **Tab**: Command/filename completion
- **Ctrl+C**: Interrupt current command
- **Ctrl+D**: Send EOF / Logout
- **Ctrl+L**: Clear screen

## Managing Connection Profiles

### Viewing Saved Profiles

All saved connection profiles are stored locally and can be accessed via:
- **File → Connection Manager** (planned feature)
- **File → Recent Connections**

### Editing Profiles

To edit a saved profile:
1. Locate the profile files in:
   - **Linux**: `~/.local/share/SSH-Client/profiles/`
   - **macOS**: `~/Library/Application Support/SSH-Client/profiles/`
   - **Windows**: `%APPDATA%\SSH-Client\profiles\`
2. Profiles are stored as JSON files

### Deleting Profiles

To remove a profile:
1. Close the application
2. Delete the corresponding `.json` file from the profiles directory
3. Or use **File → Connection Manager → Delete** (planned feature)

## Keyboard Shortcuts

### File Operations
- **Ctrl+N** (Cmd+N): New Connection
- **Ctrl+W** (Cmd+W): Close Current Tab
- **Ctrl+Q** (Cmd+Q): Quit Application

### Edit Operations
- **Ctrl+C** (Cmd+C): Copy selected text
- **Ctrl+V** (Cmd+V): Paste text
- **Ctrl+A** (Cmd+A): Select all (in terminal)

### Navigation
- **Ctrl+Tab**: Next tab
- **Ctrl+Shift+Tab**: Previous tab
- **Ctrl+1-9**: Jump to tab 1-9

## Troubleshooting

### Connection Issues

#### "Connection Refused"
- **Cause**: SSH server not running or firewall blocking port
- **Solution**:
  - Verify SSH server is running: `sudo systemctl status sshd`
  - Check firewall rules allow port 22
  - Verify hostname/IP is correct

#### "Authentication Failed"
- **Cause**: Incorrect credentials or key not authorized
- **Solution**:
  - Double-check username and password
  - For key auth, ensure public key is in `~/.ssh/authorized_keys`
  - Check key file permissions (should be 600)

#### "Host Key Verification Failed"
- **Cause**: Server's host key changed or first connection
- **Solution**:
  - For first connection, verify server fingerprint
  - If server key changed intentionally, remove old key from `~/.ssh/known_hosts`

#### "Connection Timeout"
- **Cause**: Server unreachable or network issues
- **Solution**:
  - Check network connectivity: `ping hostname`
  - Verify server is online
  - Check if VPN is required

### Application Issues

#### Application Crashes
1. Check log files:
   - **Linux**: `~/.local/share/SSH-Client/ssh-client.log`
   - **macOS**: `~/Library/Application Support/SSH-Client/ssh-client.log`
   - **Windows**: `%APPDATA%\SSH-Client\ssh-client.log`
2. Report issues with log file attached

#### Slow Terminal Performance
- Reduce scrollback buffer size in settings
- Disable complex ANSI sequences if not needed
- Check system resources (CPU/memory usage)

#### Display Issues
- Try resizing the window
- Check terminal font settings
- Ensure graphics drivers are up to date

## Advanced Features

### Log Files

Application logs are stored for debugging:
- Logs automatically rotate when they reach 10MB
- Up to 5 log files are kept
- Log level can be configured via environment variable:
  ```bash
  export QT_LOGGING_RULES="ssh.connection=true;ssh.auth=true"
  ```

### Environment Variables

Useful environment variables for testing:
- `SSH_TEST_HOST`: Default host for testing
- `SSH_TEST_USER`: Default username for testing
- `SSH_TEST_PASSWORD`: Default password for testing (insecure, only for testing!)
- `QT_LOGGING_RULES`: Control logging verbosity

### Command Line Options

Launch with specific options:
```bash
# Enable verbose logging
./ssh-client --verbose

# Specify custom config location
./ssh-client --config /path/to/config
```

(Note: Command line options are planned features)

### Multiple Concurrent Sessions

You can open multiple SSH sessions simultaneously:
1. Each session runs in its own tab
2. Tabs can be reordered by dragging
3. Close tabs with the X button or Ctrl+W
4. All sessions are independent and run in separate threads

### Session Persistence

(Planned feature) Sessions can be saved and restored:
- Save current session layout before closing
- Restore all connections on next launch
- Export/import session configurations

## Tips and Best Practices

### Security

1. **Use SSH Keys**: More secure than passwords
2. **Protect Private Keys**: Never share your private key
3. **Use Passphrases**: Protect your keys with strong passphrases
4. **Keep Software Updated**: Regularly update SSH Client and system SSH
5. **Verify Host Keys**: Always verify fingerprints on first connection

### Performance

1. **Limit Scrollback**: Reduce buffer size for better performance
2. **Close Unused Sessions**: Free up system resources
3. **Use Compression**: Enable SSH compression for slow connections (server-side)

### Workflow

1. **Save Frequently Used Servers**: Create named profiles
2. **Organize by Environment**: Use clear profile names (prod, dev, staging)
3. **Use Tab Names**: Rename tabs for easy identification
4. **Keyboard Shortcuts**: Learn shortcuts for faster workflow

## Getting Help

### Resources

- **Documentation**: Check the `docs/` directory
- **GitHub Issues**: Report bugs and request features
- **Log Files**: Include logs when reporting issues

### Reporting Issues

When reporting issues, include:
1. Application version
2. Operating system and version
3. Steps to reproduce the issue
4. Relevant log file excerpts
5. Error messages or screenshots

### Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

**Last Updated**: 2025-10-05
**Version**: 1.0.0
