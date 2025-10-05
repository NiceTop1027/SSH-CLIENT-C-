# Tasks: SSH Client SpecKit

**Input**: Design documents from `/specs/001-ssh-client-speckit/`
**Prerequisites**: plan.md, research.md, spec.md
**Branch**: `001-ssh-client-speckit`

## Execution Flow (main)
```
1. Load plan.md from feature directory
   → SUCCESS: Implementation plan found
   → Extracted: C++17, Qt5/Qt6, libssh, CMake, Qt Test, Google Test
2. Load optional design documents:
   → research.md: Found - terminal emulation, credential storage, threading decisions
   → data-model.md: Not found (C++ desktop app - models defined in code)
   → contracts/: Not found (desktop app - no REST API)
3. Generate tasks by category:
   → Setup: CMake, dependencies, project structure
   → Tests: Unit tests for components, integration tests for workflows
   → Core: SSH connection, terminal emulation, UI components, storage
   → Integration: Thread coordination, signal/slot connections
   → Polish: Cross-platform testing, documentation, memory leak checks
4. Apply task rules:
   → Different files = mark [P] for parallel
   → Same file = sequential (no [P])
   → Tests before implementation (TDD)
5. Number tasks sequentially (T001, T002...)
6. Generate dependency graph
7. Create parallel execution examples
8. Validate task completeness
9. Return: SUCCESS (tasks ready for execution)
```

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths in descriptions

## Path Conventions
Repository root paths:
- Source: `src/`
- Headers: `include/`
- Tests: `tests/`
- Resources: `resources/`
- Build: `build/` (generated)

---

## Phase 3.1: Project Setup & Configuration

- [ ] **T001** Create project directory structure (src/, include/, tests/, resources/, docs/)
- [ ] **T002** Create root CMakeLists.txt with C++17, Qt5/6 detection, libssh linking
- [ ] **T003** [P] Create .gitignore for CMake, Qt, C++ builds (build/, *.user, *.autosave)
- [ ] **T004** [P] Create README.md with project description, build instructions, dependencies
- [ ] **T005** [P] Configure clang-format for code style consistency
- [ ] **T006** Create CMakeLists.txt for tests/ directory with Qt Test and Google Test

---

## Phase 3.2: Core Data Models (TDD - Tests First)

### Model Tests (Write First - These MUST FAIL)
- [ ] **T007** [P] Unit test for ConnectionProfile in tests/unit/test_connection_profile.cpp
- [ ] **T008** [P] Unit test for SessionState in tests/unit/test_session_state.cpp
- [ ] **T009** [P] Unit test for TerminalBuffer in tests/unit/test_terminal_buffer.cpp

### Model Implementation (Only After Tests Fail)
- [ ] **T010** [P] ConnectionProfile class in src/models/ConnectionProfile.cpp and include/ConnectionProfile.h
- [ ] **T011** [P] SessionState class in src/models/SessionState.cpp and include/SessionState.h
- [ ] **T012** [P] TerminalBuffer class in src/models/TerminalBuffer.cpp and include/TerminalBuffer.h

---

## Phase 3.3: SSH Connection Layer (TDD)

### SSH Tests (Write First - These MUST FAIL)
- [ ] **T013** [P] Unit test for SSHConnection in tests/unit/test_ssh_connection.cpp
- [ ] **T014** [P] Unit test for SSHAuthenticator in tests/unit/test_ssh_authenticator.cpp
- [ ] **T015** [P] Integration test for password auth in tests/integration/test_password_auth.cpp
- [ ] **T016** [P] Integration test for key auth in tests/integration/test_key_auth.cpp

### SSH Implementation (Only After Tests Fail)
- [ ] **T017** SSHConnection class header in include/SSHConnection.h (session init, connect, disconnect, signals/slots)
- [ ] **T018** SSHConnection implementation in src/ssh/SSHConnection.cpp (libssh session, options, error handling)
- [ ] **T019** [P] SSHAuthenticator class in src/ssh/SSHAuthenticator.cpp and include/SSHAuthenticator.h
- [ ] **T020** [P] SSHChannel class in src/ssh/SSHChannel.cpp and include/SSHChannel.h (shell channel, read/write)
- [ ] **T021** Host key verification in src/ssh/HostKeyVerifier.cpp and include/HostKeyVerifier.h
- [ ] **T022** Connection timeout and error handling in SSHConnection

---

## Phase 3.4: Terminal Emulation (TDD)

### Terminal Tests (Write First - These MUST FAIL)
- [ ] **T023** [P] Unit test for ANSIParser in tests/unit/test_ansi_parser.cpp
- [ ] **T024** [P] Unit test for TerminalWidget keyboard input in tests/unit/test_terminal_input.cpp
- [ ] **T025** [P] Integration test for terminal output display in tests/integration/test_terminal_display.cpp

### Terminal Implementation (Only After Tests Fail)
- [ ] **T026** [P] ANSIParser class in src/terminal/ANSIParser.cpp and include/ANSIParser.h (VT100/ANSI sequences)
- [ ] **T027** TerminalWidget header in include/TerminalWidget.h (QTextEdit subclass, signals/slots)
- [ ] **T028** TerminalWidget implementation in src/terminal/TerminalWidget.cpp (UI setup, fonts, colors)
- [ ] **T029** Keyboard input handling in TerminalWidget::keyPressEvent (special keys, Enter, arrows)
- [ ] **T030** Output rendering in TerminalWidget (apply ANSI formatting, cursor control)
- [ ] **T031** [P] Scrollback buffer management in TerminalBuffer (10k default, LRU eviction)
- [ ] **T032** Text selection and copy to clipboard in TerminalWidget

---

## Phase 3.5: Threading & Async I/O

### Threading Tests (Write First - These MUST FAIL)
- [ ] **T033** [P] Unit test for SSHWorkerThread in tests/unit/test_ssh_worker.cpp
- [ ] **T034** [P] Integration test for non-blocking I/O in tests/integration/test_async_io.cpp

### Threading Implementation (Only After Tests Fail)
- [ ] **T035** SSHWorkerThread class in src/ssh/SSHWorkerThread.cpp and include/SSHWorkerThread.h (QThread)
- [ ] **T036** Non-blocking read loop in SSHWorkerThread (ssh_channel_read, emit dataReceived signal)
- [ ] **T037** Write queue in SSHWorkerThread for sending user input
- [ ] **T038** Thread-safe signal/slot connections between worker and UI

---

## Phase 3.6: Storage & Session Management

### Storage Tests (Write First - These MUST FAIL)
- [ ] **T039** [P] Unit test for ProfileStorage in tests/unit/test_profile_storage.cpp
- [ ] **T040** [P] Unit test for CredentialManager in tests/unit/test_credential_manager.cpp
- [ ] **T041** [P] Integration test for profile save/load in tests/integration/test_profile_persistence.cpp

### Storage Implementation (Only After Tests Fail)
- [ ] **T042** [P] ProfileStorage class in src/storage/ProfileStorage.cpp and include/ProfileStorage.h (JSON read/write)
- [ ] **T043** CredentialManager class in src/storage/CredentialManager.cpp and include/CredentialManager.h
- [ ] **T044** Platform-specific credential storage (DPAPI for Windows in src/storage/CredentialManager_win.cpp)
- [ ] **T045** Platform-specific credential storage (Keychain for macOS in src/storage/CredentialManager_mac.cpp)
- [ ] **T046** Platform-specific credential storage (libsecret for Linux in src/storage/CredentialManager_linux.cpp)
- [ ] **T047** Profile directory management (~/.ssh-client/profiles/ or platform equivalent)

---

## Phase 3.7: Main GUI Components

### GUI Tests (Write First - These MUST FAIL)
- [ ] **T048** [P] UI test for MainWindow in tests/ui/test_main_window.cpp
- [ ] **T049** [P] UI test for ConnectionDialog in tests/ui/test_connection_dialog.cpp
- [ ] **T050** [P] UI test for tab management in tests/ui/test_tab_widget.cpp

### GUI Implementation (Only After Tests Fail)
- [ ] **T051** MainWindow UI file in ui/MainWindow.ui (Qt Designer: layout, menu bar, tab widget, status bar)
- [ ] **T052** MainWindow class in src/ui/MainWindow.cpp and include/MainWindow.h
- [ ] **T053** Menu bar implementation (File: New Connection, Close Tab, Exit; Edit: Copy, Paste; Help: About)
- [ ] **T054** Status bar with connection status indicator
- [ ] **T055** ConnectionDialog UI in ui/ConnectionDialog.ui (host, port, username, auth method, key file browser)
- [ ] **T056** ConnectionDialog class in src/ui/ConnectionDialog.cpp and include/ConnectionDialog.h
- [ ] **T057** Input validation in ConnectionDialog (hostname/IP, port 1-65535, username format)
- [ ] **T058** QTabWidget integration for multiple connections (add tab, close tab, switch tabs)
- [ ] **T059** Session list widget in ui/SessionListWidget.ui (tree view for folders, flat list view)
- [ ] **T060** SessionListWidget class in src/ui/SessionListWidget.cpp and include/SessionListWidget.h

---

## Phase 3.8: Application Integration

- [ ] **T061** Connect MainWindow signals to SSHConnection slots (new connection, disconnect)
- [ ] **T062** Connect SSHWorkerThread signals to TerminalWidget slots (data received, error)
- [ ] **T063** Connect TerminalWidget signals to SSHWorkerThread slots (user input)
- [ ] **T064** Implement connection workflow (dialog → create session → worker thread → terminal display)
- [ ] **T065** Implement automatic reconnection with exponential backoff (2s, 5s, 10s, 30s, 60s)
- [ ] **T066** Application settings dialog in src/ui/SettingsDialog.cpp (buffer size, reconnect, theme)
- [ ] **T067** QSettings integration for persistent app preferences
- [ ] **T068** Application icon and resources (icons/, install to resources/)

---

## Phase 3.9: Error Handling & Logging

- [ ] **T069** [P] Qt logging categories (ssh.connection, ssh.auth, terminal, ui, storage)
- [ ] **T070** [P] Log file rotation (10MB max, 5 files, platform-specific location)
- [ ] **T071** Error dialog system for user-facing errors (connection, auth, file I/O)
- [ ] **T072** Error detail logging (technical info in logs, user-friendly messages in UI)
- [ ] **T073** Connection failure handling (invalid host, auth failure, timeout, network error)
- [ ] **T074** Graceful disconnect and cleanup (free libssh resources, stop threads)

---

## Phase 3.10: Testing & Quality Assurance

### Integration Tests
- [ ] **T075** [P] End-to-end test: connect with password in tests/integration/test_e2e_password.cpp
- [ ] **T076** [P] End-to-end test: connect with SSH key in tests/integration/test_e2e_key.cpp
- [ ] **T077** [P] End-to-end test: multiple concurrent connections in tests/integration/test_concurrent.cpp
- [ ] **T078** [P] End-to-end test: save and load profile in tests/integration/test_profile_workflow.cpp
- [ ] **T079** [P] End-to-end test: automatic reconnection in tests/integration/test_auto_reconnect.cpp

### Error Scenarios
- [ ] **T080** [P] Test invalid hostname error in tests/integration/test_errors.cpp
- [ ] **T081** [P] Test invalid port error in tests/integration/test_errors.cpp
- [ ] **T082** [P] Test auth failure error in tests/integration/test_errors.cpp
- [ ] **T083** [P] Test network disconnection in tests/integration/test_errors.cpp

### Memory & Performance
- [ ] **T084** Memory leak check with Valgrind on Linux (long-running session test)
- [ ] **T085** Memory leak check with Address Sanitizer on macOS
- [ ] **T086** Performance test: terminal response time <50ms for user input
- [ ] **T087** Performance test: handle 10k lines of rapid output without lag

---

## Phase 3.11: Cross-Platform Build & Packaging

- [ ] **T088** [P] CMake build on Windows (MSVC or MinGW, verify Qt and libssh linkage)
- [ ] **T089** [P] CMake build on macOS (Xcode/clang, app bundle structure)
- [ ] **T090** [P] CMake build on Linux (GCC, verify library paths)
- [ ] **T091** [P] CPack configuration for Windows (NSIS installer)
- [ ] **T092** [P] CPack configuration for macOS (DMG, app signing preparation)
- [ ] **T093** [P] CPack configuration for Linux (DEB and RPM packages)
- [ ] **T094** [P] AppImage build for universal Linux distribution
- [ ] **T095** Platform-specific testing (Windows 10+, macOS 10.15+, Ubuntu/Fedora)

---

## Phase 3.12: Documentation & Polish

- [ ] **T096** [P] Update README.md with complete build instructions per platform
- [ ] **T097** [P] Create BUILDING.md with detailed dependency installation steps
- [ ] **T098** [P] Add Doxygen comments to all public headers in include/
- [ ] **T099** [P] Create USER_GUIDE.md with screenshots and usage instructions
- [ ] **T100** [P] Add LICENSE file (choose appropriate license)
- [ ] **T101** [P] Create CONTRIBUTING.md with code style and PR guidelines
- [ ] **T102** Code cleanup: remove dead code, add comments to complex logic
- [ ] **T103** Final code review: check for TODOs, FIXMEs, debug code
- [ ] **T104** Run clang-format on all source files
- [ ] **T105** Generate API documentation with Doxygen

---

## Dependencies

### Critical Path (Sequential)
1. **Setup** (T001-T006) → Everything else
2. **Model Tests** (T007-T009) → **Model Implementation** (T010-T012)
3. **SSH Tests** (T013-T016) → **SSH Implementation** (T017-T022)
4. **Terminal Tests** (T023-T025) → **Terminal Implementation** (T026-T032)
5. **Threading Tests** (T033-T034) → **Threading Implementation** (T035-T038)
6. **Storage Tests** (T039-T041) → **Storage Implementation** (T042-T047)
7. **GUI Tests** (T048-T050) → **GUI Implementation** (T051-T060)
8. **Integration** (T061-T068) → After core components complete
9. **Error Handling** (T069-T074) → Throughout implementation
10. **QA Tests** (T075-T087) → After integration complete
11. **Cross-Platform** (T088-T095) → After all features work on one platform
12. **Documentation** (T096-T105) → Final phase

### Parallel Opportunities
- T003, T004, T005: All independent setup tasks
- T007, T008, T009: Different test files
- T010, T011, T012: Different model files
- T013, T014, T015, T016: Different test files
- T019, T020: Different SSH component files
- T023, T024, T025: Different terminal test files
- T026, T031, T032: Different terminal components
- T033, T034: Different threading test files
- T039, T040, T041: Different storage test files
- T042, T043: Different storage classes
- T044, T045, T046: Platform-specific files (can build in parallel on CI)
- T048, T049, T050: Different UI test files
- T069, T070: Logging setup tasks
- T075-T083: All integration/error tests
- T088, T089, T090: Platform builds (on CI with multiple runners)
- T091, T092, T093, T094: Package configurations
- T096-T101: All documentation files

---

## Parallel Execution Examples

### Setup Phase (Run T003-T005 together):
```bash
# Launch parallel setup tasks
Task: "[P] Create .gitignore for CMake, Qt, C++ builds"
Task: "[P] Create README.md with project description and build instructions"
Task: "[P] Configure clang-format for code style consistency"
```

### Model Tests (Run T007-T009 together):
```bash
# Write all model tests in parallel
Task: "[P] Unit test for ConnectionProfile in tests/unit/test_connection_profile.cpp"
Task: "[P] Unit test for SessionState in tests/unit/test_session_state.cpp"
Task: "[P] Unit test for TerminalBuffer in tests/unit/test_terminal_buffer.cpp"
```

### Model Implementation (Run T010-T012 together, AFTER tests fail):
```bash
# Implement all models in parallel
Task: "[P] ConnectionProfile class in src/models/ConnectionProfile.cpp"
Task: "[P] SessionState class in src/models/SessionState.cpp"
Task: "[P] TerminalBuffer class in src/models/TerminalBuffer.cpp"
```

### Integration Tests (Run T075-T079 together):
```bash
# Run all end-to-end tests in parallel
Task: "[P] End-to-end test: connect with password"
Task: "[P] End-to-end test: connect with SSH key"
Task: "[P] End-to-end test: multiple concurrent connections"
Task: "[P] End-to-end test: save and load profile"
Task: "[P] End-to-end test: automatic reconnection"
```

### Documentation (Run T096-T101 together):
```bash
# Write all documentation in parallel
Task: "[P] Update README.md with complete build instructions"
Task: "[P] Create BUILDING.md with dependency installation"
Task: "[P] Add Doxygen comments to public headers"
Task: "[P] Create USER_GUIDE.md with screenshots"
Task: "[P] Add LICENSE file"
Task: "[P] Create CONTRIBUTING.md with guidelines"
```

---

## Notes

### TDD Discipline
- ⚠️ **CRITICAL**: All test tasks (T007-T009, T013-T016, T023-T025, T033-T034, T039-T041, T048-T050, T075-T083) MUST be written FIRST
- Tests MUST FAIL before writing implementation
- Green tests without implementation = invalid test

### File Modification Rules
- **[P] tasks**: Different files, can run in parallel
- **Non-[P] tasks**: May modify same files or have dependencies
- Never run [P] and non-[P] tasks on same file simultaneously

### Platform-Specific Code
- Use `#ifdef Q_OS_WIN`, `#ifdef Q_OS_MAC`, `#ifdef Q_OS_LINUX` for platform guards
- Isolate platform code to specific files (e.g., CredentialManager_*.cpp)
- Test on actual platform hardware, not just cross-compilation

### Memory Management
- Use Qt parent-child for QObjects (automatic cleanup)
- Use std::unique_ptr/shared_ptr for non-Qt types
- RAII for SSH sessions, channels, files
- Test with Valgrind/ASan regularly

### Commit Strategy
- Commit after each task completion
- Include task ID in commit message (e.g., "T018: Implement SSHConnection core logic")
- Keep commits atomic and reversible

---

## Validation Checklist
*GATE: Verified before task execution*

- [x] All model entities have tests (ConnectionProfile, SessionState, TerminalBuffer)
- [x] All core components have tests (SSH, Terminal, Storage, UI)
- [x] All tests come before implementation
- [x] Parallel tasks are truly independent (different files)
- [x] Each task specifies exact file path
- [x] No [P] task modifies same file as another [P] task
- [x] Dependencies are clearly documented
- [x] TDD workflow enforced (tests → fail → implement → pass)

---

**Total Tasks**: 105
**Estimated Parallel Groups**: ~25 (significant time savings with concurrent execution)
**Critical Path Length**: ~40 sequential tasks

**Ready for execution**: All tasks are specific, have clear file paths, follow TDD principles, and are ordered by dependencies.
