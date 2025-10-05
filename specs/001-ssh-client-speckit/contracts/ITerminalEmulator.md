# Interface Contract: ITerminalEmulator

**Component**: Terminal Emulation
**Version**: 1.0
**Date**: 2025-10-05

## Purpose
Provides terminal emulation functionality including character display, ANSI escape sequence processing, scrollback buffer management, and text selection.

## Interface Definition

```cpp
class ITerminalEmulator {
public:
    virtual ~ITerminalEmulator() = default;

    // Input processing
    virtual void processInput(const std::string& input) = 0;
    virtual void processKeyEvent(const KeyEvent& event) = 0;

    // Output processing
    virtual void processOutput(const std::string& output) = 0;

    // Display state
    virtual TerminalDimensions getDimensions() const = 0;
    virtual void setDimensions(int rows, int columns) = 0;
    virtual CursorPosition getCursorPosition() const = 0;

    // Buffer management
    virtual std::vector<TerminalLine> getVisibleLines() const = 0;
    virtual std::vector<TerminalLine> getScrollbackBuffer() const = 0;
    virtual void scrollTo(int offset) = 0;
    virtual void clearBuffer() = 0;
    virtual void setMaxScrollback(int lines) = 0;

    // Text selection
    virtual void startSelection(int row, int column) = 0;
    virtual void updateSelection(int row, int column) = 0;
    virtual void endSelection() = 0;
    virtual std::string getSelectedText() const = 0;
    virtual void clearSelection() = 0;

    // State queries
    virtual bool hasSelection() const = 0;
    virtual int getScrollOffset() const = 0;
    virtual TextEncoding getEncoding() const = 0;

    // Signals (Qt-style)
    // void displayUpdated();
    // void cursorMoved(int row, int column);
    // void bellRang();
    // void dimensionsChanged(int rows, int columns);
};
```

## Types

### KeyEvent
```cpp
struct KeyEvent {
    KeyCode key;
    KeyModifiers modifiers;  // Ctrl, Alt, Shift
    std::string text;        // Character(s) typed
};
```

### TerminalDimensions
```cpp
struct TerminalDimensions {
    int rows;
    int columns;
};
```

### CursorPosition
```cpp
struct CursorPosition {
    int row;
    int column;
};
```

### TerminalLine
```cpp
struct TerminalLine {
    std::string text;
    std::vector<TextFormat> formatting;
    int lineNumber;
    bool wrapped;  // True if line wrapped from previous
};
```

### TextFormat
```cpp
struct TextFormat {
    int start;
    int end;
    bool bold;
    bool italic;
    bool underline;
    bool inverse;  // Reverse video
    Color foreground;
    Color background;
};
```

### TextEncoding
```cpp
enum class TextEncoding {
    UTF8,
    ASCII,
    Latin1
};
```

## Method Contracts

### processInput(input)
**Purpose**: Processes user input text for transmission to server

**Preconditions**:
- `input` is valid string (may be empty)

**Postconditions** (success):
- Input echoed to display (if echo enabled)
- Input buffered for retrieval

**Errors**: None (no-fail guarantee)

**Notes**:
- Local echo controlled by server settings
- Special keys (arrows, function keys) converted to escape sequences

---

### processKeyEvent(event)
**Purpose**: Processes keyboard events for special keys and shortcuts

**Preconditions**:
- `event` is valid KeyEvent structure

**Postconditions** (success):
- Special keys translated to appropriate escape sequences
- Shortcuts handled (Ctrl+C, Ctrl+V, etc.)
- Printable characters passed to processInput()

**Errors**: None (no-fail guarantee)

**Key Mappings**:
- Arrow keys → ANSI cursor movement sequences
- Page Up/Down → Scrollback navigation
- Home/End → Cursor movement or line navigation
- F1-F12 → Function key escape sequences
- Ctrl+C → Copy or interrupt (context-dependent)
- Ctrl+V → Paste

---

### processOutput(output)
**Purpose**: Processes data received from server, including ANSI escape sequences

**Preconditions**:
- `output` is valid string (may contain ANSI sequences)

**Postconditions** (success):
- Text displayed in terminal buffer
- ANSI escape sequences interpreted and applied
- Cursor position updated
- `displayUpdated` signal emitted
- Scrollback buffer updated if necessary

**Errors**: None (invalid sequences ignored gracefully)

**ANSI Support**:
- Cursor movement (CUU, CUD, CUF, CUB, CUP)
- Erase (ED, EL)
- Text formatting (SGR - colors, bold, underline, etc.)
- Scrolling regions
- Character sets (limited support)

**Unsupported** (ignored):
- Mouse tracking
- Alternate screen buffer
- Advanced VT220+ features

---

### getDimensions()
**Purpose**: Returns current terminal dimensions

**Preconditions**: None

**Postconditions**:
- Returns current terminal size in rows and columns

**Errors**: None (no-fail guarantee)

---

### setDimensions(rows, columns)
**Purpose**: Updates terminal dimensions (on window resize)

**Preconditions**:
- `rows` > 0 and <= 200
- `columns` > 0 and <= 500

**Postconditions** (success):
- Terminal dimensions updated
- Display reflows to new size
- Cursor adjusted if beyond new bounds
- `dimensionsChanged` signal emitted

**Errors**:
- `INVALID_DIMENSIONS`: Row/column values out of range

---

### getCursorPosition()
**Purpose**: Returns current cursor position

**Preconditions**: None

**Postconditions**:
- Returns cursor row and column (0-based)

**Errors**: None (no-fail guarantee)

---

### getVisibleLines()
**Purpose**: Returns currently visible terminal lines

**Preconditions**: None

**Postconditions**:
- Returns vector of TerminalLine for visible portion
- Size = terminal rows

**Errors**: None (no-fail guarantee)

---

### getScrollbackBuffer()
**Purpose**: Returns entire scrollback buffer

**Preconditions**: None

**Postconditions**:
- Returns vector of all lines in buffer (visible + scrollback)
- Ordered from oldest to newest

**Errors**: None (no-fail guarantee)

**Performance**: O(n) where n = buffer size, use sparingly

---

### scrollTo(offset)
**Purpose**: Scrolls display to specified offset

**Preconditions**:
- `offset` >= 0 and <= (buffer size - visible rows)

**Postconditions** (success):
- Display scrolled to offset
- 0 = showing most recent lines (bottom)
- Max offset = showing oldest lines (top)
- `displayUpdated` signal emitted

**Errors**:
- `INVALID_OFFSET`: Offset out of range (clamped to valid range)

---

### clearBuffer()
**Purpose**: Clears all terminal content and scrollback

**Preconditions**: None

**Postconditions**:
- All lines removed from buffer
- Cursor reset to (0, 0)
- Display cleared
- `displayUpdated` signal emitted

**Errors**: None (no-fail guarantee)

---

### setMaxScrollback(lines)
**Purpose**: Sets maximum scrollback buffer size

**Preconditions**:
- `lines` >= 100 and <= 100,000

**Postconditions** (success):
- Scrollback limit updated
- If current buffer > new limit, oldest lines discarded

**Errors**:
- `INVALID_SCROLLBACK`: Value out of range

---

### startSelection(row, column)
**Purpose**: Begins text selection at specified position

**Preconditions**:
- `row`, `column` within buffer bounds

**Postconditions**:
- Selection mode activated
- Selection anchor set to (row, column)

**Errors**: None (invalid positions clamped)

---

### updateSelection(row, column)
**Purpose**: Updates selection endpoint as user drags

**Preconditions**:
- Selection mode active (startSelection called)
- `row`, `column` within buffer bounds

**Postconditions**:
- Selection endpoint updated
- `displayUpdated` signal emitted (to show selection highlight)

**Errors**: None (invalid positions clamped)

---

### endSelection()
**Purpose**: Finalizes text selection

**Preconditions**:
- Selection mode active

**Postconditions**:
- Selection finalized
- Selected text available via getSelectedText()

**Errors**: None (no-op if no selection active)

---

### getSelectedText()
**Purpose**: Returns currently selected text

**Preconditions**: None

**Postconditions**:
- Returns selected text as string
- Empty string if no selection

**Errors**: None (no-fail guarantee)

**Note**: Text extracted with line breaks preserved

---

### clearSelection()
**Purpose**: Clears current text selection

**Preconditions**: None

**Postconditions**:
- Selection cleared
- `displayUpdated` signal emitted

**Errors**: None (no-fail guarantee)

---

### hasSelection()
**Purpose**: Checks if text is currently selected

**Preconditions**: None

**Postconditions**:
- Returns true if selection active, false otherwise

**Errors**: None (no-fail guarantee)

---

### getScrollOffset()
**Purpose**: Returns current scroll offset

**Preconditions**: None

**Postconditions**:
- Returns offset from bottom (0 = at bottom)

**Errors**: None (no-fail guarantee)

---

### getEncoding()
**Purpose**: Returns current text encoding

**Preconditions**: None

**Postconditions**:
- Returns current encoding (usually UTF8)

**Errors**: None (no-fail guarantee)

---

## Signals

### displayUpdated()
**Emitted when**: Terminal display content changes

**Payload**: None

**Purpose**: Notify UI to repaint terminal widget

---

### cursorMoved(row, column)
**Emitted when**: Cursor position changes

**Payload**:
- row: New cursor row
- column: New cursor column

**Purpose**: Update cursor rendering in UI

---

### bellRang()
**Emitted when**: Bell character (BEL, \x07) received

**Payload**: None

**Purpose**: Trigger audio/visual bell notification

---

### dimensionsChanged(rows, columns)
**Emitted when**: Terminal dimensions updated

**Payload**:
- rows: New row count
- columns: New column count

**Purpose**: Notify server of new terminal size

---

## ANSI Escape Sequence Support

### Supported Sequences

**Cursor Control**:
- `ESC[<n>A` - Cursor Up
- `ESC[<n>B` - Cursor Down
- `ESC[<n>C` - Cursor Forward
- `ESC[<n>D` - Cursor Backward
- `ESC[<row>;<col>H` - Cursor Position

**Erase**:
- `ESC[J` - Erase Display
- `ESC[K` - Erase Line

**Graphics Mode (SGR)**:
- `ESC[0m` - Reset
- `ESC[1m` - Bold
- `ESC[4m` - Underline
- `ESC[7m` - Reverse Video
- `ESC[30-37m` - Foreground Colors
- `ESC[40-47m` - Background Colors
- `ESC[90-97m` - Bright Foreground Colors
- `ESC[100-107m` - Bright Background Colors

### Unsupported (Gracefully Ignored)
- Mouse tracking
- Alternate screen
- Programmable keys
- Status reports

---

## Performance Requirements

- `processOutput()`: Process 1MB/sec of data
- `getVisibleLines()`: < 1ms for typical terminal size (24x80)
- ANSI parsing: < 100μs per escape sequence
- Display update latency: < 16ms (60 FPS capable)

---

## Test Scenarios

1. **Basic text display**: processOutput("Hello") → text visible
2. **ANSI colors**: processOutput("\\x1b[31mRed") → red text
3. **Cursor movement**: processOutput("\\x1b[5;10H") → cursor at (5,10)
4. **Line wrapping**: Long line → wraps at column limit
5. **Scrollback**: Full buffer → oldest line removed
6. **Text selection**: startSelection → updateSelection → getSelectedText
7. **Clear buffer**: clearBuffer() → empty display
8. **Dimension change**: setDimensions → display reflows
9. **Special keys**: Arrow key → correct escape sequence
10. **High-throughput**: 1MB output → no dropped characters

---

**Status**: Contract defined, ready for test generation
