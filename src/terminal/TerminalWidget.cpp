#include "TerminalWidget.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QPainter>
#include <QTimer>

TerminalWidget::TerminalWidget(QWidget* parent)
    : QTextEdit(parent), m_rows(24), m_columns(80), m_enableFileColoring(true)
{
    setupTerminal();
}

TerminalWidget::~TerminalWidget()
{
}

void TerminalWidget::setupTerminal()
{
    setupFont();

    // Configure text edit for terminal use
    setLineWrapMode(QTextEdit::NoWrap);
    setReadOnly(false); // Allow direct text manipulation
    setUndoRedoEnabled(false);
    setAcceptRichText(false);

    // Enable text selection with mouse
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    // Set background and foreground colors
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(0, 0, 0));
    p.setColor(QPalette::Text, QColor(170, 170, 170));

    // Set selection colors for better visibility
    p.setColor(QPalette::Highlight, QColor(51, 153, 255));
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    setPalette(p);

    // Enable proper opaque rendering to prevent ghosting
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(true);

    // Optimize viewport updates - opaque to prevent transparency artifacts
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    viewport()->setAutoFillBackground(true);

    // Enable mouse tracking for better selection
    setMouseTracking(true);

    // Set initial buffer dimensions
    m_buffer = TerminalBuffer(m_rows, m_columns);
}

void TerminalWidget::setupFont()
{
    // Use a monospace font
#ifdef Q_OS_MAC
    m_terminalFont = QFont("Monaco", 12);
#elif defined(Q_OS_WIN)
    m_terminalFont = QFont("Consolas", 10);
#else
    m_terminalFont = QFont("Monospace", 10);
#endif

    m_terminalFont.setFixedPitch(true);
    setFont(m_terminalFont);
}

void TerminalWidget::displayOutput(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    // Batch updates to reduce paint events
    setUpdatesEnabled(false);

    // Check for clear screen command
    if (text.contains("\x1b[2J") || text.contains("\x1b[H\x1b[2J")) {
        // Force complete clear with immediate repaint
        clear();
        viewport()->update();
        repaint();

        // Remove the clear sequence from text
        QString remaining = text;
        remaining.remove("\x1b[2J");
        remaining.remove("\x1b[H");
        if (remaining.trimmed().isEmpty()) {
            setUpdatesEnabled(true);
            return;
        }
    }

    // Apply file coloring only to ls-like output (not user input echo)
    // Only colorize if it looks like ls output (multiple columns or detailed listing)
    QString processedText = text;
    if (m_enableFileColoring && !text.contains("\x1b[")) {
        // Check if this looks like ls output
        bool looksLikeLsOutput = false;
        QStringList lines = text.split('\n');
        for (const QString& line : lines) {
            // Check for ls -l pattern (starts with permissions like drwxr-xr-x)
            if (line.length() > 10 &&
                (line[0] == 'd' || line[0] == '-' || line[0] == 'l') &&
                (line[1] == 'r' || line[1] == '-') &&
                (line[2] == 'w' || line[2] == '-')) {
                looksLikeLsOutput = true;
                break;
            }
            // Check for multi-column output (3+ space-separated items)
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                // Check if items have file extensions
                int filesWithExt = 0;
                for (const QString& part : parts) {
                    if (part.contains('.') && !part.startsWith('.')) {
                        filesWithExt++;
                    }
                }
                if (filesWithExt >= 2) {
                    looksLikeLsOutput = true;
                    break;
                }
            }
        }

        if (looksLikeLsOutput) {
            processedText = m_fileColorizer.colorizeLsOutput(text);
        }
    }

    // Parse ANSI codes with colors
    auto tokens = parseAnsiWithColors(processedText);

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);

    // Process each token with its formatting
    for (const auto& token : tokens) {
        // Apply text format
        QTextCharFormat format;

        if (token.fgColor.isValid()) {
            format.setForeground(token.fgColor);
        }
        if (token.bgColor.isValid()) {
            format.setBackground(token.bgColor);
        }
        if (token.bold) {
            format.setFontWeight(QFont::Bold);
        }

        cursor.setCharFormat(format);

        // Insert text with control character handling
        for (int i = 0; i < token.text.length(); ++i) {
            QChar ch = token.text[i];

            if (ch == '\b' || ch == QChar(0x7f)) {
                if (!cursor.atBlockStart()) {
                    cursor.deletePreviousChar();
                }
            } else if (ch == '\r') {
                if (i + 1 < token.text.length() && token.text[i + 1] == '\n') {
                    cursor.insertText("\n");
                    i++;
                } else {
                    cursor.movePosition(QTextCursor::StartOfBlock);
                }
            } else if (ch == '\n') {
                cursor.insertText("\n");
            } else if (ch.isPrint() || ch == ' ' || ch == '\t') {
                cursor.insertText(QString(ch));
            }
        }
    }

    cursor.endEditBlock();
    setTextCursor(cursor);

    setUpdatesEnabled(true);
    ensureCursorVisible();
}

QVector<TerminalWidget::ColorToken> TerminalWidget::parseAnsiWithColors(const QString& text)
{
    QVector<ColorToken> tokens;
    ColorToken current;
    int i = 0;

    while (i < text.length()) {
        if (text[i] == '\x1b' && i + 1 < text.length()) {
            QChar next = text[i + 1];

            if (next == '[') {
                // Save current text
                if (!current.text.isEmpty()) {
                    tokens.append(current);
                    current.text.clear();
                }

                // Parse CSI sequence
                i += 2;
                QString params;
                while (i < text.length() && !text[i].isLetter()) {
                    params += text[i];
                    i++;
                }

                if (i < text.length() && text[i] == 'J') {
                    // ED - Erase in Display (handled in displayOutput)
                    // Skip this sequence
                } else if (i < text.length() && text[i] == 'H') {
                    // CUP - Cursor Position (handled in displayOutput)
                    // Skip this sequence
                } else if (i < text.length() && text[i] == 'K') {
                    // EL - Erase in Line (handled in displayOutput)
                    // Skip this sequence
                } else if (i < text.length() && text[i] == 'm') {
                    // SGR - color/style codes
                    QStringList parts = params.split(';');

                    for (int j = 0; j < parts.size(); ++j) {
                        int code = parts[j].toInt();

                        if (code == 0) {
                            // Reset
                            current.fgColor = QColor();
                            current.bgColor = QColor();
                            current.bold = false;
                        } else if (code == 1) {
                            current.bold = true;
                        } else if (code == 22) {
                            current.bold = false;
                        } else if (code >= 30 && code <= 37) {
                            // Foreground colors
                            current.fgColor = getAnsiColor(code - 30, false);
                        } else if (code == 38 && j + 2 < parts.size()) {
                            // Extended foreground color
                            int type = parts[j + 1].toInt();
                            if (type == 5 && j + 2 < parts.size()) {
                                // 256 color
                                int colorIndex = parts[j + 2].toInt();
                                current.fgColor = get256Color(colorIndex);
                                j += 2;
                            } else if (type == 2 && j + 4 < parts.size()) {
                                // RGB color
                                int r = parts[j + 2].toInt();
                                int g = parts[j + 3].toInt();
                                int b = parts[j + 4].toInt();
                                current.fgColor = QColor(r, g, b);
                                j += 4;
                            }
                        } else if (code == 39) {
                            // Default foreground
                            current.fgColor = QColor();
                        } else if (code >= 40 && code <= 47) {
                            // Background colors
                            current.bgColor = getAnsiColor(code - 40, false);
                        } else if (code == 48 && j + 2 < parts.size()) {
                            // Extended background color
                            int type = parts[j + 1].toInt();
                            if (type == 5 && j + 2 < parts.size()) {
                                // 256 color
                                int colorIndex = parts[j + 2].toInt();
                                current.bgColor = get256Color(colorIndex);
                                j += 2;
                            } else if (type == 2 && j + 4 < parts.size()) {
                                // RGB color
                                int r = parts[j + 2].toInt();
                                int g = parts[j + 3].toInt();
                                int b = parts[j + 4].toInt();
                                current.bgColor = QColor(r, g, b);
                                j += 4;
                            }
                        } else if (code == 49) {
                            // Default background
                            current.bgColor = QColor();
                        } else if (code >= 90 && code <= 97) {
                            // Bright foreground colors
                            current.fgColor = getAnsiColor(code - 90, true);
                        } else if (code >= 100 && code <= 107) {
                            // Bright background colors
                            current.bgColor = getAnsiColor(code - 100, true);
                        }
                    }
                }
                i++;
            } else if (next == ']') {
                // OSC sequence - skip
                i += 2;
                while (i < text.length()) {
                    if (text[i] == '\x07') {
                        i++;
                        break;
                    }
                    if (text[i] == '\x1b' && i + 1 < text.length() && text[i + 1] == '\\') {
                        i += 2;
                        break;
                    }
                    i++;
                }
            } else if (next == '(' || next == ')' || next == '>' || next == '=' || next == '<') {
                // Skip other escape sequences
                i += 2;
                if (i < text.length() && !text[i-1].isLetter()) {
                    i++;
                }
            } else {
                i += 2;
            }
        } else {
            current.text += text[i];
            i++;
        }
    }

    if (!current.text.isEmpty()) {
        tokens.append(current);
    }

    return tokens;
}

QColor TerminalWidget::getAnsiColor(int colorCode, bool bright)
{
    if (bright) {
        switch (colorCode) {
            case 0: return QColor(85, 85, 85);      // Bright Black (Gray)
            case 1: return QColor(255, 85, 85);     // Bright Red
            case 2: return QColor(85, 255, 85);     // Bright Green
            case 3: return QColor(255, 255, 85);    // Bright Yellow
            case 4: return QColor(85, 85, 255);     // Bright Blue
            case 5: return QColor(255, 85, 255);    // Bright Magenta
            case 6: return QColor(85, 255, 255);    // Bright Cyan
            case 7: return QColor(255, 255, 255);   // Bright White
        }
    } else {
        switch (colorCode) {
            case 0: return QColor(0, 0, 0);         // Black
            case 1: return QColor(170, 0, 0);       // Red
            case 2: return QColor(0, 170, 0);       // Green
            case 3: return QColor(170, 85, 0);      // Yellow
            case 4: return QColor(0, 0, 170);       // Blue
            case 5: return QColor(170, 0, 170);     // Magenta
            case 6: return QColor(0, 170, 170);     // Cyan
            case 7: return QColor(170, 170, 170);   // White
        }
    }
    return QColor();
}

QColor TerminalWidget::get256Color(int index)
{
    // 0-15: Standard colors
    if (index < 16) {
        if (index < 8) {
            return getAnsiColor(index, false);
        } else {
            return getAnsiColor(index - 8, true);
        }
    }

    // 16-231: 216 color cube (6x6x6)
    if (index >= 16 && index <= 231) {
        int colorIndex = index - 16;
        int r = (colorIndex / 36) * 51;
        int g = ((colorIndex / 6) % 6) * 51;
        int b = (colorIndex % 6) * 51;
        return QColor(r, g, b);
    }

    // 232-255: Grayscale
    if (index >= 232 && index <= 255) {
        int gray = 8 + (index - 232) * 10;
        return QColor(gray, gray, gray);
    }

    return QColor();
}

QString TerminalWidget::getDisplayedText() const
{
    return toPlainText();
}

void TerminalWidget::setDimensions(int rows, int columns)
{
    m_rows = rows;
    m_columns = columns;
    m_buffer.setDimensions(rows, columns);
    emit dimensionsChanged(rows, columns);
}

int TerminalWidget::rows() const
{
    return m_rows;
}

int TerminalWidget::columns() const
{
    return m_columns;
}

void TerminalWidget::clearDisplay()
{
    // Force immediate update to prevent ghosting
    setUpdatesEnabled(false);
    clear();
    m_buffer.clear();

    // Force complete repaint with background
    viewport()->update();
    setUpdatesEnabled(true);
    repaint();
}

void TerminalWidget::setMaxScrollback(int lines)
{
    m_buffer.setMaxScrollback(lines);
}

void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    // Allow Cmd+C for copy on macOS
    if (event->matches(QKeySequence::Copy)) {
        copy();
        event->accept();
        return;
    }

    // Allow Cmd+V for paste on macOS
    if (event->matches(QKeySequence::Paste)) {
        QString clipText = QApplication::clipboard()->text();
        if (!clipText.isEmpty()) {
            emit sendData(clipText);
        }
        event->accept();
        return;
    }

    // Allow Cmd+A for select all on macOS
    if (event->matches(QKeySequence::SelectAll)) {
        selectAll();
        event->accept();
        return;
    }

    handleKeyInput(event);
    event->accept();
}

void TerminalWidget::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);

    // Calculate new dimensions based on widget size
    QFontMetrics fm(m_terminalFont);
    int charWidth = fm.horizontalAdvance('M');
    int charHeight = fm.height();

    int newCols = width() / charWidth;
    int newRows = height() / charHeight;

    if (newCols > 0 && newRows > 0 && (newCols != m_columns || newRows != m_rows)) {
        setDimensions(newRows, newCols);
    }
}

void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    // Enable text selection with mouse
    if (event->button() == Qt::LeftButton) {
        QTextEdit::mousePressEvent(event);
    } else if (event->button() == Qt::RightButton) {
        // Right click for context menu (copy/paste)
        if (textCursor().hasSelection()) {
            copy();
        } else {
            // Paste if no selection
            QString clipText = QApplication::clipboard()->text();
            if (!clipText.isEmpty()) {
                emit sendData(clipText);
            }
        }
        event->accept();
    } else {
        QTextEdit::mousePressEvent(event);
    }
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Auto-copy on selection (like traditional terminals)
        if (textCursor().hasSelection()) {
            copy();
        }
    }
    QTextEdit::mouseReleaseEvent(event);
}

void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Allow drag selection
    QTextEdit::mouseMoveEvent(event);
}

void TerminalWidget::processANSIOutput(const QString& text)
{
    // Parse ANSI codes and extract tokens
    auto tokens = m_ansiParser.parseToTokens(text);

    if (tokens.isEmpty()) {
        // No formatting, just plain text
        QStringList lines = text.split('\n');
        for (const QString& line : lines) {
            m_buffer.appendLine(line);
        }
        return;
    }

    // Process tokens with formatting
    for (const auto& token : tokens) {
        QStringList lines = token.text.split('\n');
        for (int i = 0; i < lines.size(); i++) {
            if (!lines[i].isEmpty() || i < lines.size() - 1) {
                m_buffer.appendLine(lines[i]);
            }
        }
        m_currentFormat = token.format;
    }
}

void TerminalWidget::applyTextFormat(const ANSIParser::TextFormat& format)
{
    QTextCharFormat charFormat;

    // Apply bold
    if (format.bold) {
        charFormat.setFontWeight(QFont::Bold);
    }

    // Apply italic
    if (format.italic) {
        charFormat.setFontItalic(true);
    }

    // Apply underline
    if (format.underline) {
        charFormat.setFontUnderline(true);
    }

    // Apply colors
    if (format.foregroundColor != ANSIParser::Color::Default) {
        charFormat.setForeground(ANSIParser::toQColor(format.foregroundColor));
    }

    if (format.backgroundColor != ANSIParser::Color::Default) {
        charFormat.setBackground(ANSIParser::toQColor(format.backgroundColor));
    }

    // Apply inverse (swap fg/bg)
    if (format.inverse) {
        QColor fg = charFormat.foreground().color();
        QColor bg = charFormat.background().color();
        charFormat.setForeground(bg);
        charFormat.setBackground(fg);
    }

    textCursor().setCharFormat(charFormat);
}

void TerminalWidget::handleKeyInput(QKeyEvent* event)
{
    QString data = keyEventToString(event);

    if (!data.isEmpty()) {
        emit sendData(data);
    }
}

QString TerminalWidget::keyEventToString(QKeyEvent* event)
{
    int key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Handle Ctrl combinations
    if (modifiers & Qt::ControlModifier) {
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            // Ctrl+A = 0x01, Ctrl+B = 0x02, etc.
            int ctrlChar = key - Qt::Key_A + 1;
            return QString(QChar(ctrlChar));
        }

        // Special Ctrl combinations
        switch (key) {
        case Qt::Key_Space:
            return QString(QChar(0x00)); // Ctrl+Space = NUL
        case Qt::Key_BracketLeft:
            return QString(QChar(0x1B)); // Ctrl+[ = ESC
        case Qt::Key_Backslash:
            return QString(QChar(0x1C)); // Ctrl+\ = FS
        case Qt::Key_BracketRight:
            return QString(QChar(0x1D)); // Ctrl+] = GS
        case Qt::Key_AsciiCircum:
            return QString(QChar(0x1E)); // Ctrl+^ = RS
        case Qt::Key_Underscore:
            return QString(QChar(0x1F)); // Ctrl+_ = US
        }
    }

    // Handle arrow keys
    switch (key) {
    case Qt::Key_Up:
        return "\x1b[A";
    case Qt::Key_Down:
        return "\x1b[B";
    case Qt::Key_Right:
        return "\x1b[C";
    case Qt::Key_Left:
        return "\x1b[D";
    }

    // Handle function keys
    switch (key) {
    case Qt::Key_F1:
        return "\x1bOP";
    case Qt::Key_F2:
        return "\x1bOQ";
    case Qt::Key_F3:
        return "\x1bOR";
    case Qt::Key_F4:
        return "\x1bOS";
    case Qt::Key_F5:
        return "\x1b[15~";
    case Qt::Key_F6:
        return "\x1b[17~";
    case Qt::Key_F7:
        return "\x1b[18~";
    case Qt::Key_F8:
        return "\x1b[19~";
    case Qt::Key_F9:
        return "\x1b[20~";
    case Qt::Key_F10:
        return "\x1b[21~";
    case Qt::Key_F11:
        return "\x1b[23~";
    case Qt::Key_F12:
        return "\x1b[24~";
    }

    // Handle special keys
    switch (key) {
    case Qt::Key_Home:
        return "\x1b[H";
    case Qt::Key_End:
        return "\x1b[F";
    case Qt::Key_Insert:
        return "\x1b[2~";
    case Qt::Key_Delete:
        return "\x1b[3~";
    case Qt::Key_PageUp:
        return "\x1b[5~";
    case Qt::Key_PageDown:
        return "\x1b[6~";
    case Qt::Key_Backspace:
        return "\x7f"; // DEL character
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return "\r"; // Only CR for SSH
    case Qt::Key_Tab:
        return "\t";
    case Qt::Key_Escape:
        return "\x1b";
    }

    // Handle printable characters
    if (!event->text().isEmpty()) {
        return event->text();
    }

    return QString();
}

void TerminalWidget::updateDisplayFromBuffer()
{
    // Get visible lines from buffer
    auto lines = m_buffer.getVisibleLines();

    // Update display
    QString displayText;
    for (const QString& line : lines) {
        displayText += line + "\n";
    }

    // Preserve scroll position
    QScrollBar* vbar = verticalScrollBar();
    bool wasAtBottom = (vbar->value() == vbar->maximum());

    setPlainText(displayText);

    // Auto-scroll to bottom if we were at bottom
    if (wasAtBottom) {
        vbar->setValue(vbar->maximum());
    }
}

void TerminalWidget::setFileColoringEnabled(bool enabled)
{
    m_enableFileColoring = enabled;
}

bool TerminalWidget::isFileColoringEnabled() const
{
    return m_enableFileColoring;
}

void TerminalWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* menu = new QMenu(this);

    QAction* copyAction = menu->addAction("Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(textCursor().hasSelection());
    connect(copyAction, &QAction::triggered, this, &TerminalWidget::copy);

    QAction* pasteAction = menu->addAction("Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(!QApplication::clipboard()->text().isEmpty());
    connect(pasteAction, &QAction::triggered, this, [this]() {
        QString clipText = QApplication::clipboard()->text();
        if (!clipText.isEmpty()) {
            emit sendData(clipText);
        }
    });

    menu->addSeparator();

    QAction* selectAllAction = menu->addAction("Select All");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &TerminalWidget::selectAll);

    menu->addSeparator();

    QAction* clearAction = menu->addAction("Clear");
    connect(clearAction, &QAction::triggered, this, &TerminalWidget::clearDisplay);

    menu->exec(event->globalPos());
    delete menu;
}

void TerminalWidget::paintEvent(QPaintEvent* event)
{
    // Fill background completely to prevent ghosting
    QPainter painter(viewport());
    painter.fillRect(event->rect(), QColor(0, 0, 0));

    // Call base implementation with optimized rendering
    QTextEdit::paintEvent(event);
}
