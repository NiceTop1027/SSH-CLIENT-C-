# Quickstart Guide: SSH Client SpecKit

**Version**: 1.0
**Date**: 2025-10-05

## Purpose
This guide provides step-by-step instructions to validate that the SSH client application meets all functional requirements. It represents the "happy path" user journey and serves as acceptance criteria.

## Prerequisites

### For Testing
- **Test SSH Server**: Local SSH server or remote test server with known credentials
  - Option 1: Local OpenSSH server (Linux/macOS: `sudo systemctl start sshd`)
  - Option 2: Docker container: `docker run -d -p 2222:22 rastasheep/ubuntu-sshd` (user: root, password: root)
- **Test SSH Key**: Key pair for SSH key authentication testing
  - Generate if needed: `ssh-keygen -t rsa -f ~/.ssh/test_key -N ""`
  - Add public key to test server: `ssh-copy-id -i ~/.ssh/test_key.pub user@testserver`

### Application Setup
- Application installed and launched
- Internet/network connectivity to test server
- Write permissions for config directory

---

## Test Scenario 1: First-Time Password Connection

**Goal**: Connect to SSH server using password authentication

### Steps

1. **Launch Application**
   - Start the SSH client application
   - **Expected**: Main window opens with empty connection list

2. **Open Connection Dialog**
   - Click "New Connection" button or File → New Connection
   - **Expected**: Connection dialog appears

3. **Enter Connection Details**
   - Name: "Test Server - Password"
   - Hostname: `localhost` (or your test server)
   - Port: `22` (or `2222` for Docker test server)
   - Username: `root` (or your test username)
   - Authentication: Select "Password"
   - **Expected**: All fields accept input, port defaults to 22

4. **Save Profile**
   - Click "Save Profile" checkbox (optional)
   - Click "Connect" button
   - **Expected**: Password prompt appears

5. **Enter Password**
   - Enter password for test user
   - Click "OK" or press Enter
   - **Expected**: Connection attempt begins, status shows "Connecting..."

6. **Verify Connection**
   - **Expected**:
     - Status changes to "Connected" (green indicator)
     - Terminal area becomes active and shows server banner/prompt
     - Status bar shows "Connected to Test Server - Password"

7. **Verify Terminal Functionality**
   - Type: `echo "Hello, SSH!"`
   - Press Enter
   - **Expected**: Command echoed, "Hello, SSH!" displayed, new prompt appears

---

## Test Scenario 2: SSH Key Authentication

**Goal**: Connect using SSH private key

### Steps

1. **Create New Connection**
   - Click "New Connection"
   - Name: "Test Server - Key Auth"
   - Hostname: Same as Test 1
   - Port: Same as Test 1
   - Username: Same as Test 1
   - Authentication: Select "SSH Key"
   - **Expected**: SSH Key file selector appears

2. **Select SSH Key**
   - Click "Browse" next to SSH Key Path
   - Navigate to `~/.ssh/test_key`
   - Select private key file
   - **Expected**: Key path displayed in field

3. **Connect with Key**
   - Click "Connect"
   - If key is encrypted: Enter passphrase
   - **Expected**: Connection succeeds without password prompt

4. **Verify Connection**
   - **Expected**: Same as Test 1, step 6

---

## Test Scenario 3: Saved Profile Usage

**Goal**: Verify saved profiles can be reused

### Steps

1. **Disconnect Active Connection**
   - Click "Disconnect" button or File → Disconnect
   - **Expected**: Connection closes, status shows "Disconnected"

2. **View Saved Profiles**
   - **Expected**: Profiles list shows "Test Server - Password" and "Test Server - Key Auth"

3. **Connect from Saved Profile**
   - Double-click "Test Server - Key Auth" in profiles list
   - **Expected**: Connection dialog pre-filled with saved details

4. **Quick Connect**
   - Click "Connect"
   - **Expected**: Connection established without re-entering details

---

## Test Scenario 4: Terminal Interaction

**Goal**: Verify terminal emulation and interaction features

### Steps

1. **Connected State**
   - Use an active connection from previous tests

2. **Basic Commands**
   - Execute: `pwd` → current directory shown
   - Execute: `ls` → directory listing shown
   - Execute: `uname -a` → system info shown
   - **Expected**: All output displayed correctly in terminal

3. **Long Output**
   - Execute: `dmesg` or `cat /var/log/syslog | head -100`
   - **Expected**: Long output scrolls, scrollback preserves earlier lines

4. **Test Scrollback**
   - Scroll up with mouse wheel or Page Up
   - **Expected**: Can scroll through previous output
   - Scroll to bottom
   - **Expected**: Returns to current output

5. **Text Selection**
   - Drag mouse to select text in terminal
   - **Expected**: Selected text highlighted

6. **Copy Text**
   - With text selected, press Ctrl+C (or Cmd+C on macOS)
   - **Expected**: Text copied to clipboard (verify by pasting elsewhere)

7. **Color Output**
   - Execute: `ls --color=auto` (Linux) or use colored output tool
   - **Expected**: Colored text displayed (if supported by server)

---

## Test Scenario 5: Multiple Concurrent Connections

**Goal**: Verify support for multiple simultaneous SSH connections

### Steps

1. **First Connection Active**
   - Ensure one connection is active from previous tests

2. **Open Second Connection**
   - Click "New Connection" or "Connect" on different saved profile
   - Connect to same or different server
   - **Expected**: Second connection opens in new tab

3. **Verify Tab Interface**
   - **Expected**:
     - Two tabs visible (Tab 1: first connection, Tab 2: second connection)
     - Each tab shows connection name
     - Active tab indicated visually

4. **Switch Between Connections**
   - Click Tab 1
   - Type command: `echo "Tab 1"`
   - Click Tab 2
   - Type command: `echo "Tab 2"`
   - **Expected**: Each terminal operates independently

5. **Close Single Connection**
   - Right-click Tab 1 → Close, or click X on tab
   - **Expected**: Tab 1 closes, Tab 2 remains active

---

## Test Scenario 6: Profile Management

**Goal**: Verify profile CRUD operations

### Steps

1. **Edit Existing Profile**
   - Right-click saved profile → Edit
   - Change name to "Test Server - Updated"
   - Click "Save"
   - **Expected**: Profile name updated in list

2. **Create Profile with Folder**
   - Create new profile
   - Set Folder field to "Development"
   - Save profile
   - **Expected**: Profile appears under "Development" folder in tree view

3. **Delete Profile**
   - Right-click profile → Delete
   - Confirm deletion
   - **Expected**: Profile removed from list, file deleted

4. **Search Profiles**
   - Enter "test" in search box
   - **Expected**: Profiles filtered to show only matches

---

## Test Scenario 7: Connection Error Handling

**Goal**: Verify error handling for connection failures

### Steps

1. **Invalid Hostname**
   - Create connection with hostname: `invalid.server.notexist`
   - Attempt to connect
   - **Expected**: Error dialog: "Connection failed: Unable to resolve hostname"

2. **Wrong Port**
   - Create connection with port: `2323` (assuming nothing listening)
   - Attempt to connect
   - **Expected**: Error dialog: "Connection failed: Connection refused" or timeout

3. **Wrong Password**
   - Connect with password authentication
   - Enter incorrect password
   - **Expected**: Error dialog: "Authentication failed: Invalid password"

4. **Wrong SSH Key**
   - Connect with key authentication
   - Select key not authorized on server
   - **Expected**: Error dialog: "Authentication failed: Key not accepted"

5. **Error Recovery**
   - After error, correct the issue and retry
   - **Expected**: Subsequent connection succeeds

---

## Test Scenario 8: Application Settings

**Goal**: Verify global settings configuration

### Steps

1. **Open Settings**
   - Click Edit → Settings or Preferences
   - **Expected**: Settings dialog opens

2. **Change Default Values**
   - Set default port to `2222`
   - Set default scrollback to `5000`
   - Save settings
   - **Expected**: Settings persisted

3. **Verify Defaults Applied**
   - Create new connection profile
   - **Expected**: Port field defaults to `2222`

4. **Change Theme** (if implemented)
   - Switch theme to Dark/Light
   - **Expected**: UI updates to reflect theme

---

## Test Scenario 9: Auto-Reconnect

**Goal**: Verify automatic reconnection functionality

### Steps

1. **Enable Auto-Reconnect**
   - Edit profile, check "Enable auto-reconnect"
   - Save profile and connect

2. **Simulate Connection Loss**
   - On server, kill SSH session: `pkill -9 sshd` (if test server)
   - Or disconnect network temporarily
   - **Expected**:
     - Status shows "Connection lost"
     - Status changes to "Reconnecting... (attempt 1 of 5)"

3. **Verify Reconnection**
   - Restore connectivity (if simulated)
   - **Expected**: Connection re-established automatically

4. **Verify Reconnection Failure**
   - Keep server unreachable
   - **Expected**: After 5 attempts, status shows "Disconnected - reconnection failed"

---

## Test Scenario 10: Cross-Platform Verification

**Goal**: Verify application works on target platforms

### Steps (Repeat on Windows, macOS, Linux)

1. **Install Application**
   - Install platform-specific package
   - **Expected**: Installation succeeds

2. **Launch Application**
   - Start application from menu/desktop
   - **Expected**: Main window appears with native look

3. **Basic Connection Test**
   - Create and connect to test server
   - **Expected**: Connection works as on other platforms

4. **Settings Location**
   - Verify config files in platform-specific location:
     - Windows: `%APPDATA%\SSHClient`
     - macOS: `~/Library/Application Support/SSHClient`
     - Linux: `~/.config/ssh-client`
   - **Expected**: Files created in correct location

---

## Test Scenario 11: Memory and Performance

**Goal**: Verify application resource usage

### Steps

1. **Long-Running Session**
   - Connect to server
   - Leave connection active for 30 minutes
   - Occasionally run commands
   - **Expected**: Memory usage remains stable (no leaks)

2. **High-Throughput Output**
   - Execute: `find /` or similar command producing lots of output
   - **Expected**:
     - Output displays without freezing UI
     - Scrollback buffer respects limit
     - No excessive memory usage

3. **Multiple Connections**
   - Open 10 concurrent connections
   - **Expected**:
     - All connections remain responsive
     - Memory usage scales linearly (not exponentially)

---

## Acceptance Criteria

All test scenarios must pass for acceptance:

- ✅ First-time password connection works
- ✅ SSH key authentication works
- ✅ Saved profiles can be reused
- ✅ Terminal displays output correctly
- ✅ Text selection and copy work
- ✅ Multiple concurrent connections supported
- ✅ Profile CRUD operations work
- ✅ Connection errors handled gracefully
- ✅ Settings persist and apply
- ✅ Auto-reconnect functions correctly (when enabled)
- ✅ Application works on all target platforms
- ✅ No memory leaks in long-running sessions
- ✅ UI remains responsive during operations

---

## Cleanup

After testing:

1. **Delete Test Profiles**
   - Remove all test profiles created during testing

2. **Stop Test Server**
   - If using Docker: `docker stop <container_id>`
   - If using local SSH: No action needed

3. **Remove Test Keys**
   - Delete test SSH keys if created specifically for testing

---

## Success Metrics

**Definition of Done**:
- All 11 test scenarios completed successfully
- Zero critical bugs found
- All acceptance criteria met
- Application performs within specified constraints
- Cross-platform verification complete

---

**Status**: Quickstart guide complete, ready for implementation and testing
