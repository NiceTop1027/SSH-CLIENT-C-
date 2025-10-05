#include "TerminalEmulator.h"
#include <QDebug>

TerminalEmulator::TerminalEmulator(int rows, int cols)
    : m_screen(rows, cols)
    , m_state(State::Normal)
{
}

void TerminalEmulator::processData(const QString& data)
{
    for (const QChar& ch : data) {
        processChar(ch);
    }
}

void TerminalEmulator::processData(const QByteArray& data)
{
    processData(QString::fromUtf8(data));
}

void TerminalEmulator::resize(int rows, int cols)
{
    m_screen.resize(rows, cols);
}

void TerminalEmulator::processChar(QChar ch)
{
    switch (m_state) {
    case State::Normal:
        if (ch == '\x1b') {  // ESC
            m_state = State::Escape;
            m_escapeBuffer.clear();
        } else if (ch == '\n') {
            m_screen.newLine();
        } else if (ch == '\r') {
            m_screen.carriageReturn();
        } else if (ch == '\b' || ch == QChar(0x7f)) {
            m_screen.backspace();
        } else if (ch == '\t') {
            m_screen.tab();
        } else if (ch == '\a') {
            // Bell - ignore for now
        } else if (ch.isPrint() || ch == ' ') {
            m_screen.putChar(ch);
        }
        break;

    case State::Escape:
        if (ch == '[') {
            m_state = State::CSI;
            m_escapeBuffer.clear();
        } else if (ch == ']') {
            m_state = State::OSC;
            m_oscString.clear();
        } else if (ch == 'M') {
            // Reverse index (scroll down)
            if (m_screen.cursorRow() > 0) {
                m_screen.setCursorPos(m_screen.cursorRow() - 1, m_screen.cursorCol());
            } else {
                m_screen.scrollDown();
            }
            m_state = State::Normal;
        } else if (ch == 'D') {
            // Index (scroll up)
            m_screen.newLine();
            m_state = State::Normal;
        } else if (ch == 'E') {
            // Next line
            m_screen.newLine();
            m_state = State::Normal;
        } else if (ch == '7') {
            // Save cursor position (DECSC)
            // TODO: implement cursor save/restore
            m_state = State::Normal;
        } else if (ch == '8') {
            // Restore cursor position (DECRC)
            // TODO: implement cursor save/restore
            m_state = State::Normal;
        } else if (ch == '(') {
            // Character set selection - skip next char
            m_state = State::Normal;
        } else {
            m_state = State::Normal;
        }
        break;

    case State::CSI:
        if (ch.isDigit() || ch == ';' || ch == '?') {
            m_escapeBuffer += ch;
        } else {
            handleCSI();
            executeCsiCommand(ch, {});
            m_state = State::Normal;
        }
        break;

    case State::OSC:
        if (ch == '\x07' || ch == '\x1b') {  // BEL or ESC
            // OSC sequence complete
            m_state = State::Normal;
        } else {
            m_oscString += ch;
        }
        break;

    case State::OSC_String:
        // Not used currently
        m_state = State::Normal;
        break;
    }
}

void TerminalEmulator::handleCSI()
{
    // This function is no longer needed - parsing moved to executeCsiCommand
}

void TerminalEmulator::executeCsiCommand(QChar command, const QVector<int>&)
{
    // Parse parameters from m_escapeBuffer
    QVector<int> params;
    QStringList parts = m_escapeBuffer.split(';');

    bool hasQuestionMark = m_escapeBuffer.startsWith('?');
    if (hasQuestionMark && !parts.isEmpty()) {
        parts[0] = parts[0].mid(1);  // Remove '?'
    }

    for (const QString& part : parts) {
        if (!part.isEmpty()) {
            params.append(part.toInt());
        }
    }

    if (params.isEmpty()) {
        params.append(0);
    }

    switch (command.toLatin1()) {
    case 'A':  // CUU - Cursor Up
        m_screen.setCursorPos(m_screen.cursorRow() - std::max(1, params[0]), m_screen.cursorCol());
        break;

    case 'B':  // CUD - Cursor Down
        m_screen.setCursorPos(m_screen.cursorRow() + std::max(1, params[0]), m_screen.cursorCol());
        break;

    case 'C':  // CUF - Cursor Forward
        m_screen.setCursorPos(m_screen.cursorRow(), m_screen.cursorCol() + std::max(1, params[0]));
        break;

    case 'D':  // CUB - Cursor Back
        m_screen.setCursorPos(m_screen.cursorRow(), m_screen.cursorCol() - std::max(1, params[0]));
        break;

    case 'E':  // CNL - Cursor Next Line
        m_screen.setCursorPos(m_screen.cursorRow() + std::max(1, params[0]), 0);
        break;

    case 'F':  // CPL - Cursor Previous Line
        m_screen.setCursorPos(m_screen.cursorRow() - std::max(1, params[0]), 0);
        break;

    case 'G':  // CHA - Cursor Horizontal Absolute
        m_screen.setCursorPos(m_screen.cursorRow(), params[0] - 1);
        break;

    case 'H':  // CUP - Cursor Position
    case 'f':  // HVP - Horizontal Vertical Position
    {
        int row = (params.size() > 0 && params[0] > 0) ? params[0] - 1 : 0;
        int col = (params.size() > 1 && params[1] > 0) ? params[1] - 1 : 0;
        m_screen.setCursorPos(row, col);
        break;
    }

    case 'J':  // ED - Erase in Display
        if (params[0] == 0) {
            m_screen.clearFromCursorToEnd();
        } else if (params[0] == 1) {
            m_screen.clearFromCursorToBeginning();
        } else if (params[0] == 2 || params[0] == 3) {
            m_screen.clearScreen();
        }
        break;

    case 'K':  // EL - Erase in Line
        if (params[0] == 0) {
            m_screen.clearLineFromCursor();
        } else if (params[0] == 1) {
            m_screen.clearLineToCursor();
        } else if (params[0] == 2) {
            m_screen.clearLine();
        }
        break;

    case 'L':  // IL - Insert Line
        m_screen.scrollDown(std::max(1, params[0]));
        break;

    case 'M':  // DL - Delete Line
        m_screen.scrollUp(std::max(1, params[0]));
        break;

    case 'P':  // DCH - Delete Character
        // TODO: implement
        break;

    case 'S':  // SU - Scroll Up
        m_screen.scrollUp(std::max(1, params[0]));
        break;

    case 'T':  // SD - Scroll Down
        m_screen.scrollDown(std::max(1, params[0]));
        break;

    case 'r':  // DECSTBM - Set scrolling region
        if (params.size() >= 2) {
            m_screen.setScrollRegion(params[0] - 1, params[1] - 1);
        } else {
            m_screen.resetScrollRegion();
        }
        break;

    case 'h':  // SM - Set Mode
        if (hasQuestionMark) {
            for (int param : params) {
                if (param == 25) {
                    m_screen.setCursorVisible(true);
                } else if (param == 1049 || param == 47) {
                    m_screen.useAlternateBuffer();
                }
            }
        }
        break;

    case 'l':  // RM - Reset Mode
        if (hasQuestionMark) {
            for (int param : params) {
                if (param == 25) {
                    m_screen.setCursorVisible(false);
                } else if (param == 1049 || param == 47) {
                    m_screen.useNormalBuffer();
                }
            }
        }
        break;

    case 'm':  // SGR - Select Graphic Rendition
    {
        for (int i = 0; i < params.size(); ++i) {
            int code = params[i];

            if (code == 0) {
                m_screen.resetAttributes();
            } else if (code == 1) {
                m_screen.setBold(true);
            } else if (code == 3) {
                m_screen.setItalic(true);
            } else if (code == 4) {
                m_screen.setUnderline(true);
            } else if (code == 7) {
                m_screen.setInverse(true);
            } else if (code == 22) {
                m_screen.setBold(false);
            } else if (code == 23) {
                m_screen.setItalic(false);
            } else if (code == 24) {
                m_screen.setUnderline(false);
            } else if (code == 27) {
                m_screen.setInverse(false);
            } else if (code >= 30 && code <= 37) {
                m_screen.setFgColor(getAnsiColor(code - 30, false));
            } else if (code == 38) {
                // Extended foreground color
                if (i + 2 < params.size() && params[i + 1] == 5) {
                    m_screen.setFgColor(get256Color(params[i + 2]));
                    i += 2;
                } else if (i + 4 < params.size() && params[i + 1] == 2) {
                    m_screen.setFgColor(QColor(params[i + 2], params[i + 3], params[i + 4]));
                    i += 4;
                }
            } else if (code == 39) {
                m_screen.setFgColor(QColor(170, 170, 170));
            } else if (code >= 40 && code <= 47) {
                m_screen.setBgColor(getAnsiColor(code - 40, false));
            } else if (code == 48) {
                // Extended background color
                if (i + 2 < params.size() && params[i + 1] == 5) {
                    m_screen.setBgColor(get256Color(params[i + 2]));
                    i += 2;
                } else if (i + 4 < params.size() && params[i + 1] == 2) {
                    m_screen.setBgColor(QColor(params[i + 2], params[i + 3], params[i + 4]));
                    i += 4;
                }
            } else if (code == 49) {
                m_screen.setBgColor(QColor());
            } else if (code >= 90 && code <= 97) {
                m_screen.setFgColor(getAnsiColor(code - 90, true));
            } else if (code >= 100 && code <= 107) {
                m_screen.setBgColor(getAnsiColor(code - 100, true));
            }
        }
        break;
    }

    default:
        // Unknown command, ignore
        break;
    }
}

QColor TerminalEmulator::getAnsiColor(int colorCode, bool bright) const
{
    if (bright) {
        switch (colorCode) {
            case 0: return QColor(85, 85, 85);
            case 1: return QColor(255, 85, 85);
            case 2: return QColor(85, 255, 85);
            case 3: return QColor(255, 255, 85);
            case 4: return QColor(85, 85, 255);
            case 5: return QColor(255, 85, 255);
            case 6: return QColor(85, 255, 255);
            case 7: return QColor(255, 255, 255);
        }
    } else {
        switch (colorCode) {
            case 0: return QColor(0, 0, 0);
            case 1: return QColor(170, 0, 0);
            case 2: return QColor(0, 170, 0);
            case 3: return QColor(170, 85, 0);
            case 4: return QColor(0, 0, 170);
            case 5: return QColor(170, 0, 170);
            case 6: return QColor(0, 170, 170);
            case 7: return QColor(170, 170, 170);
        }
    }
    return QColor(170, 170, 170);
}

QColor TerminalEmulator::get256Color(int index) const
{
    if (index < 16) {
        return getAnsiColor(index % 8, index >= 8);
    }

    if (index >= 16 && index <= 231) {
        int colorIndex = index - 16;
        int r = (colorIndex / 36) * 51;
        int g = ((colorIndex / 6) % 6) * 51;
        int b = (colorIndex % 6) * 51;
        return QColor(r, g, b);
    }

    if (index >= 232 && index <= 255) {
        int gray = 8 + (index - 232) * 10;
        return QColor(gray, gray, gray);
    }

    return QColor(170, 170, 170);
}
