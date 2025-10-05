# Contributing to SSH Client

Thank you for your interest in contributing to SSH Client! This document provides guidelines and instructions for contributing.

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Process](#development-process)
4. [Coding Standards](#coding-standards)
5. [Testing Guidelines](#testing-guidelines)
6. [Pull Request Process](#pull-request-process)
7. [Issue Reporting](#issue-reporting)

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inclusive experience for everyone. We expect all contributors to:

- Be respectful and considerate
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

### Unacceptable Behavior

- Harassment, discrimination, or offensive comments
- Personal attacks or trolling
- Publishing others' private information
- Any conduct which could reasonably be considered inappropriate

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Development Environment**:
   - C++17 compliant compiler (GCC 7+, Clang 5+, MSVC 2017+)
   - CMake 3.16+
   - Qt 5.15+ or Qt 6
   - libssh development libraries
   - Git

2. **Knowledge**:
   - C++ programming (STL, modern C++ features)
   - Qt framework basics
   - SSH protocol fundamentals
   - Git version control

### Setting Up Development Environment

1. **Fork the Repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/ssh-client.git
   cd ssh-client
   ```

2. **Add Upstream Remote**
   ```bash
   git remote add upstream https://github.com/ORIGINAL_OWNER/ssh-client.git
   ```

3. **Install Dependencies**
   ```bash
   # Linux
   sudo apt-get install build-essential cmake qt6-base-dev libssh-dev

   # macOS
   brew install cmake qt libssh

   # Windows
   # Use vcpkg or install manually
   ```

4. **Build the Project**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ctest
   ```

## Development Process

### Branching Strategy

We use a feature branch workflow:

1. **Main Branches**:
   - `main`: Stable, production-ready code
   - `develop`: Integration branch for features

2. **Feature Branches**:
   - Create from `develop`
   - Name format: `feature/short-description`
   - Example: `feature/add-sftp-support`

3. **Bug Fix Branches**:
   - Create from `main` for hotfixes
   - Name format: `bugfix/issue-number-description`
   - Example: `bugfix/123-connection-timeout`

### Workflow

1. **Create a Branch**
   ```bash
   git checkout develop
   git pull upstream develop
   git checkout -b feature/your-feature-name
   ```

2. **Make Changes**
   - Write code following our standards
   - Add tests for new functionality
   - Update documentation as needed

3. **Commit Changes**
   ```bash
   git add .
   git commit -m "Add feature: Brief description"
   ```

4. **Keep Branch Updated**
   ```bash
   git fetch upstream
   git rebase upstream/develop
   ```

5. **Push to Your Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create Pull Request**
   - Go to GitHub and create a PR
   - Fill in the PR template
   - Link related issues

## Coding Standards

### C++ Style Guide

We follow a modified LLVM coding style (configured in `.clang-format`):

#### File Organization

```cpp
// 1. System includes
#include <QString>
#include <QObject>

// 2. Library includes
#include <libssh/libssh.h>

// 3. Project includes
#include "MyClass.h"
#include "Logger.h"

// 4. Class definition
class MyClass : public QObject {
    Q_OBJECT
public:
    // Public types
    enum class State { ... };

    // Constructors
    explicit MyClass(QObject* parent = nullptr);
    ~MyClass();

    // Public methods
    void publicMethod();

signals:
    void signalName();

private slots:
    void onSlotName();

private:
    // Private methods
    void privateMethod();

    // Member variables (m_ prefix)
    QString m_data;
    int m_count;
};
```

#### Naming Conventions

- **Classes**: `PascalCase` (e.g., `ConnectionProfile`)
- **Functions/Methods**: `camelCase` (e.g., `connectToHost()`)
- **Member Variables**: `m_camelCase` (e.g., `m_session`)
- **Constants**: `UPPER_CASE` or `kPascalCase` (e.g., `MAX_RETRIES`, `kDefaultPort`)
- **Namespaces**: `lowercase` (if used)

#### Best Practices

1. **Modern C++**:
   ```cpp
   // Use auto for complex types
   auto connection = std::make_unique<SSHConnection>();

   // Use range-based for loops
   for (const auto& profile : profiles) { ... }

   // Use nullptr instead of NULL
   ssh_session session = nullptr;

   // Use override and final
   void method() override;
   ```

2. **Qt Idioms**:
   ```cpp
   // Use Q_OBJECT macro for QObject subclasses
   class MyClass : public QObject {
       Q_OBJECT
   };

   // Use signals and slots
   connect(sender, &Sender::signal, receiver, &Receiver::slot);

   // Use Qt containers when interoperating with Qt APIs
   QVector<QString> items;

   // Use tr() for translatable strings
   QString message = tr("Connection failed");
   ```

3. **Resource Management**:
   ```cpp
   // Use RAII and smart pointers
   std::unique_ptr<SSHConnection> conn(new SSHConnection());

   // For Qt objects with parents, raw pointers are ok
   auto* widget = new QWidget(parent); // parent manages lifetime

   // Always cleanup in destructors
   ~MyClass() {
       if (m_resource) {
           cleanup(m_resource);
       }
   }
   ```

4. **Error Handling**:
   ```cpp
   // Return bool for success/failure
   bool performAction() {
       if (!precondition()) {
           qWarning() << "Precondition failed";
           return false;
       }
       return true;
   }

   // Use enums for error types
   enum class ErrorType { None, Network, Auth, Unknown };

   // Log errors appropriately
   qCritical(sshConnection) << "Connection failed:" << error;
   ```

### Qt-Specific Guidelines

1. **Signals and Slots**:
   - Use new-style connect syntax
   - Avoid auto-connections (prefer explicit)
   - Document signal emissions

2. **Memory Management**:
   - Leverage Qt parent-child relationship
   - Use `QObject::deleteLater()` for deferred deletion
   - Be careful with threads

3. **Threading**:
   - Use `QThread` for background work
   - Communicate via signals/slots
   - Protect shared data with `QMutex`

### Code Formatting

We use `clang-format` for automatic formatting:

```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all source files
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

Configuration is in `.clang-format` at the project root.

## Testing Guidelines

### Test-Driven Development

We follow TDD principles:

1. **Write Tests First**:
   - Write failing test
   - Implement minimum code to pass
   - Refactor while keeping tests green

2. **Test Coverage**:
   - Aim for >80% code coverage
   - Test both success and failure paths
   - Include edge cases

### Test Structure

```cpp
#include "MyClass.h"
#include <QTest>

class TestMyClass : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();      // Run once before all tests
    void cleanupTestCase();   // Run once after all tests
    void init();              // Run before each test
    void cleanup();           // Run after each test

    // Test cases
    void testDefaultConstructor();
    void testMethodWithValidInput();
    void testMethodWithInvalidInput();
    void testSignalEmission();
};

void TestMyClass::testMethodWithValidInput()
{
    // Arrange
    MyClass object;
    QString input = "test";

    // Act
    bool result = object.processInput(input);

    // Assert
    QVERIFY(result);
    QCOMPARE(object.getValue(), expectedValue);
}

QTEST_MAIN(TestMyClass)
#include "test_my_class.moc"
```

### Testing Types

1. **Unit Tests**: Test individual classes/methods
   - Located in `tests/unit/`
   - Fast execution
   - No external dependencies

2. **Integration Tests**: Test component interactions
   - Located in `tests/integration/`
   - May require test SSH server
   - Environment variables for configuration

3. **UI Tests**: Test GUI components
   - Located in `tests/ui/`
   - Use `QTest::mouseClick`, `QTest::keyClick`, etc.

4. **Performance Tests**: Benchmark critical paths
   - Located in `tests/performance/`
   - Use `QBENCHMARK`
   - Document baseline performance

### Running Tests

```bash
# All tests
ctest

# Specific test suite
./tests/unit/test_connection_profile

# With verbose output
ctest --verbose

# Integration tests (requires setup)
export SSH_TEST_HOST=localhost
export SSH_TEST_USER=testuser
./tests/integration/test_e2e_connection

# Memory leak checks
./scripts/check_memory_leaks.sh
```

## Pull Request Process

### Before Submitting

1. **Code Quality**:
   - [ ] Code follows style guidelines
   - [ ] All tests pass
   - [ ] New tests added for new features
   - [ ] No compiler warnings
   - [ ] Code formatted with clang-format

2. **Documentation**:
   - [ ] Public APIs documented
   - [ ] README updated if needed
   - [ ] CHANGELOG updated

3. **Commits**:
   - [ ] Commits are logical and atomic
   - [ ] Commit messages are clear and descriptive
   - [ ] No merge commits (rebase instead)

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] Manual testing performed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings
- [ ] Tests pass locally

## Related Issues
Closes #123
```

### Review Process

1. **Automated Checks**:
   - CI/CD builds must pass
   - Tests must pass
   - Code coverage must not decrease

2. **Manual Review**:
   - At least one approval required
   - Address all review comments
   - Update PR based on feedback

3. **Merging**:
   - Squash merge for feature branches
   - Fast-forward merge for hotfixes
   - Delete branch after merge

## Issue Reporting

### Bug Reports

Use this template:

```markdown
**Describe the Bug**
Clear description of the bug

**To Reproduce**
1. Go to '...'
2. Click on '....'
3. See error

**Expected Behavior**
What should happen

**Actual Behavior**
What actually happens

**Environment**
- OS: [e.g., Ubuntu 22.04]
- Qt Version: [e.g., 6.4.2]
- App Version: [e.g., 1.0.0]

**Logs**
Attach relevant log file excerpts

**Screenshots**
If applicable
```

### Feature Requests

```markdown
**Feature Description**
Clear description of proposed feature

**Use Case**
Why is this feature needed?

**Proposed Solution**
How should this work?

**Alternatives Considered**
Other approaches you've thought about

**Additional Context**
Mockups, examples, etc.
```

## Development Tips

### Debugging

1. **Enable Debug Logging**:
   ```cpp
   qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] [%{category}] %{message}");
   ```

2. **Qt Creator Debugging**:
   - Use Qt Creator IDE for best Qt debugging experience
   - Visualizers for Qt types built-in

3. **GDB/LLDB**:
   ```bash
   gdb ./ssh-client
   (gdb) run
   (gdb) bt  # backtrace on crash
   ```

### Common Pitfalls

1. **Memory Leaks**:
   - Always check Qt object parent relationships
   - Use smart pointers for non-QObject classes
   - Run valgrind/ASAN regularly

2. **Threading Issues**:
   - Never access GUI from non-main thread
   - Use `QMetaObject::invokeMethod` for cross-thread calls
   - Protect shared data with mutexes

3. **Signal/Slot**:
   - Check connection return value
   - Use `Qt::QueuedConnection` for cross-thread
   - Disconnect in destructor if needed

## Getting Help

- **Questions**: Open a GitHub Discussion
- **Bugs**: Create an Issue
- **Security**: Email security@example.com (do not create public issues)

## License

By contributing, you agree that your contributions will be licensed under the project's license.

---

Thank you for contributing to SSH Client!
