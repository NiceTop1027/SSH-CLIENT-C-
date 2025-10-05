
# Implementation Plan: SSH Client SpecKit

**Branch**: `001-ssh-client-speckit` | **Date**: 2025-10-05 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-ssh-client-speckit/spec.md`

## Execution Flow (/plan command scope)
```
1. Load feature spec from Input path
   → If not found: ERROR "No feature spec at {path}"
2. Fill Technical Context (scan for NEEDS CLARIFICATION)
   → Detect Project Type from file system structure or context (web=frontend+backend, mobile=app+api)
   → Set Structure Decision based on project type
3. Fill the Constitution Check section based on the content of the constitution document.
4. Evaluate Constitution Check section below
   → If violations exist: Document in Complexity Tracking
   → If no justification possible: ERROR "Simplify approach first"
   → Update Progress Tracking: Initial Constitution Check
5. Execute Phase 0 → research.md
   → If NEEDS CLARIFICATION remain: ERROR "Resolve unknowns"
6. Execute Phase 1 → contracts, data-model.md, quickstart.md, agent-specific template file (e.g., `CLAUDE.md` for Claude Code, `.github/copilot-instructions.md` for GitHub Copilot, `GEMINI.md` for Gemini CLI, `QWEN.md` for Qwen Code, or `AGENTS.md` for all other agents).
7. Re-evaluate Constitution Check section
   → If new violations: Refactor design, return to Phase 1
   → Update Progress Tracking: Post-Design Constitution Check
8. Plan Phase 2 → Describe task generation approach (DO NOT create tasks.md)
9. STOP - Ready for /tasks command
```

**IMPORTANT**: The /plan command STOPS at step 7. Phases 2-4 are executed by other commands:
- Phase 2: /tasks command creates tasks.md
- Phase 3-4: Implementation execution (manual or via tools)

## Summary
A cross-platform desktop SSH client that enables system administrators to securely connect to remote servers, manage connection profiles, and interact with remote systems through a terminal interface. The application provides both password and SSH key authentication, session management with saved profiles, real-time terminal emulation with standard features (scrolling, text selection/copying), and connection status monitoring. User-provided technical context indicates a C++/Qt implementation with libssh and CMake build system.

## Technical Context
**Language/Version**: C++17 or later
**Primary Dependencies**: Qt5/Qt6 (GUI framework), libssh (SSH protocol implementation)
**Storage**: Local file system for session profiles and configuration; NEEDS CLARIFICATION on credential storage security
**Testing**: Qt Test framework, Google Test (for non-Qt components)
**Target Platform**: Cross-platform desktop (Windows 10+, macOS 10.15+, Linux with X11/Wayland)
**Project Type**: Single desktop application
**Performance Goals**: Terminal response <50ms for user input, real-time output streaming, UI responsiveness during network operations
**Constraints**: Memory-safe operation (no leaks), secure credential handling, responsive UI (non-blocking network I/O), terminal buffer management for long-running sessions
**Scale/Scope**: Single-user application, support for concurrent connections (NEEDS CLARIFICATION: max concurrent connections), saved session profiles (NEEDS CLARIFICATION: max profiles), terminal emulation level (NEEDS CLARIFICATION: VT100/xterm compatibility)

## Constitution Check
*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Status**: No project-specific constitution found (template only exists)

**Default Checks Applied**:
- ✅ **Single Application Scope**: This is a standalone desktop application, no additional projects needed
- ✅ **Simplicity First**: Using established libraries (Qt, libssh) rather than custom implementations
- ⚠️ **Testing Strategy**: TDD approach required - tests must be written before implementation
- ⚠️ **Memory Safety**: C++ requires explicit attention to RAII, smart pointers, and leak prevention
- ⚠️ **Cross-Platform Consistency**: Qt abstracts platform differences, but testing on all platforms required

**Deviations**: None at this stage

## Project Structure

### Documentation (this feature)
```
specs/[###-feature]/
├── plan.md              # This file (/plan command output)
├── research.md          # Phase 0 output (/plan command)
├── data-model.md        # Phase 1 output (/plan command)
├── quickstart.md        # Phase 1 output (/plan command)
├── contracts/           # Phase 1 output (/plan command)
└── tasks.md             # Phase 2 output (/tasks command - NOT created by /plan)
```

### Source Code (repository root)
```
src/
├── models/              # Data models (ConnectionProfile, Session, etc.)
├── ssh/                 # SSH connection management and protocol handling
├── terminal/            # Terminal emulation and display
├── ui/                  # Qt GUI components (MainWindow, dialogs, widgets)
├── storage/             # Session profile persistence
└── utils/               # Common utilities and helpers

include/
└── [public headers for major components]

tests/
├── unit/                # Unit tests for individual components
├── integration/         # Integration tests for feature workflows
└── fixtures/            # Test data and mock servers

resources/
├── icons/               # Application icons and UI assets
├── styles/              # Qt stylesheets (if applicable)
└── translations/        # Internationalization files (future)

docs/
└── [API documentation, architecture notes]

CMakeLists.txt           # Root build configuration
```

**Structure Decision**: Single desktop application structure. The codebase is organized by functional layer (models, SSH logic, terminal, UI) rather than feature modules, which is appropriate for a cohesive desktop application. Qt-specific components are isolated in `ui/`, while core SSH and terminal logic remain UI-framework independent for better testability.

## Phase 0: Outline & Research
1. **Extract unknowns from Technical Context** above:
   - For each NEEDS CLARIFICATION → research task
   - For each dependency → best practices task
   - For each integration → patterns task

2. **Generate and dispatch research agents**:
   ```
   For each unknown in Technical Context:
     Task: "Research {unknown} for {feature context}"
   For each technology choice:
     Task: "Find best practices for {tech} in {domain}"
   ```

3. **Consolidate findings** in `research.md` using format:
   - Decision: [what was chosen]
   - Rationale: [why chosen]
   - Alternatives considered: [what else evaluated]

**Output**: research.md with all NEEDS CLARIFICATION resolved

## Phase 1: Design & Contracts
*Prerequisites: research.md complete*

1. **Extract entities from feature spec** → `data-model.md`:
   - Entity name, fields, relationships
   - Validation rules from requirements
   - State transitions if applicable

2. **Generate API contracts** from functional requirements:
   - For each user action → endpoint
   - Use standard REST/GraphQL patterns
   - Output OpenAPI/GraphQL schema to `/contracts/`

3. **Generate contract tests** from contracts:
   - One test file per endpoint
   - Assert request/response schemas
   - Tests must fail (no implementation yet)

4. **Extract test scenarios** from user stories:
   - Each story → integration test scenario
   - Quickstart test = story validation steps

5. **Update agent file incrementally** (O(1) operation):
   - Run `.specify/scripts/bash/update-agent-context.sh claude`
     **IMPORTANT**: Execute it exactly as specified above. Do not add or remove any arguments.
   - If exists: Add only NEW tech from current plan
   - Preserve manual additions between markers
   - Update recent changes (keep last 3)
   - Keep under 150 lines for token efficiency
   - Output to repository root

**Output**: data-model.md, /contracts/*, failing tests, quickstart.md, agent-specific file

## Phase 2: Task Planning Approach
*This section describes what the /tasks command will do - DO NOT execute during /plan*

**Task Generation Strategy**:
1. **Infrastructure Setup** (5 tasks)
   - CMake build system configuration
   - Qt project structure setup
   - libssh integration and linking
   - Test framework setup (Qt Test + Google Test)
   - CI/CD pipeline basics (build verification)

2. **Data Models** (5 tasks, mostly parallel)
   - ConnectionProfile model implementation [P]
   - ConnectionSession model implementation [P]
   - TerminalBuffer model implementation [P]
   - AuthenticationCredential model implementation [P]
   - ApplicationSettings model implementation [P]

3. **Core Components - Contract Tests First** (15 tasks, TDD order)
   - IConnectionManager contract tests
   - IConnectionManager implementation
   - ITerminalEmulator contract tests
   - ITerminalEmulator ANSI parser implementation
   - ITerminalEmulator display logic implementation
   - IProfileStorage contract tests
   - IProfileStorage file I/O implementation
   - IProfileStorage validation logic
   - OS credential manager integration (Windows DPAPI)
   - OS credential manager integration (macOS Keychain)
   - OS credential manager integration (Linux libsecret)
   - SSH worker thread implementation
   - Signal/slot connection layer
   - Integration tests for SSH connection flow
   - Integration tests for terminal I/O flow

4. **UI Components** (12 tasks)
   - MainWindow skeleton with Qt Designer
   - Connection dialog UI
   - Terminal widget (extending QTextEdit)
   - Profile list widget (tree and list views)
   - Settings dialog
   - Menu bar and actions
   - Status bar
   - Tab management for multiple connections
   - Error dialog component
   - UI event handlers connecting to core components
   - Keyboard event processing for terminal
   - Mouse event processing for text selection

5. **Integration & Polish** (8 tasks)
   - End-to-end integration tests (quickstart scenarios)
   - Profile import/export functionality
   - Auto-reconnect logic
   - Platform-specific packaging (Windows)
   - Platform-specific packaging (macOS)
   - Platform-specific packaging (Linux)
   - Memory leak testing with Valgrind/ASAN
   - Performance testing (terminal throughput)

**Ordering Strategy**:
- **Phase-based**: Infrastructure → Models → Core (TDD) → UI → Integration
- **TDD mandatory**: Contract tests before implementation for all core components
- **Parallel opportunities**: Mark [P] for tasks within same phase that don't depend on each other
- **Platform-specific**: Cross-platform tests pass before platform-specific packaging

**Dependency Chains**:
1. CMake setup → All other tasks
2. Models complete → Core component implementation
3. Core components complete → UI components
4. UI complete → Integration tests
5. Integration tests pass → Packaging

**Estimated Output**: ~45 numbered, dependency-ordered tasks in tasks.md

**IMPORTANT**: This phase is executed by the /tasks command, NOT by /plan

## Phase 3+: Future Implementation
*These phases are beyond the scope of the /plan command*

**Phase 3**: Task execution (/tasks command creates tasks.md)  
**Phase 4**: Implementation (execute tasks.md following constitutional principles)  
**Phase 5**: Validation (run tests, execute quickstart.md, performance validation)

## Complexity Tracking
*Fill ONLY if Constitution Check has violations that must be justified*

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |


## Progress Tracking
*This checklist is updated during execution flow*

**Phase Status**:
- [x] Phase 0: Research complete (/plan command)
- [x] Phase 1: Design complete (/plan command)
- [x] Phase 2: Task planning complete (/plan command - describe approach only)
- [ ] Phase 3: Tasks generated (/tasks command)
- [ ] Phase 4: Implementation complete
- [ ] Phase 5: Validation passed

**Gate Status**:
- [x] Initial Constitution Check: PASS (no project constitution, defaults applied)
- [x] Post-Design Constitution Check: PASS (no violations detected)
- [x] All NEEDS CLARIFICATION resolved (addressed in research.md)
- [x] Complexity deviations documented (none required)

**Generated Artifacts**:
- ✅ research.md - Technical decisions and best practices
- ✅ data-model.md - Entity definitions and relationships
- ✅ contracts/IConnectionManager.md - SSH connection interface contract
- ✅ contracts/ITerminalEmulator.md - Terminal emulation interface contract
- ✅ contracts/IProfileStorage.md - Profile persistence interface contract
- ✅ quickstart.md - Acceptance test scenarios
- ✅ CLAUDE.md - Agent context file (updated)

**Ready for**: `/tasks` command to generate tasks.md

---
*Based on Constitution template - No project-specific constitution defined*
