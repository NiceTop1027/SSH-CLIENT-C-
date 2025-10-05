#include "TerminalScreen.h"
#include <algorithm>

TerminalScreen::TerminalScreen(int rows, int cols)
    : m_rows(rows)
    , m_cols(cols)
    , m_cursorRow(0)
    , m_cursorCol(0)
    , m_cursorVisible(true)
    , m_useAlternate(false)
    , m_currentFg(QColor(170, 170, 170))
    , m_currentBg(QColor())
    , m_currentBold(false)
    , m_currentUnderline(false)
    , m_currentInverse(false)
    , m_currentItalic(false)
    , m_scrollTop(0)
    , m_scrollBottom(rows - 1)
{
    initBuffer(m_normalBuffer);
    initBuffer(m_alternateBuffer);
}

void TerminalScreen::initBuffer(QVector<QVector<Cell>>& buffer)
{
    buffer.clear();
    buffer.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        buffer[i].resize(m_cols);
    }
}

void TerminalScreen::resize(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    m_scrollBottom = rows - 1;

    initBuffer(m_normalBuffer);
    initBuffer(m_alternateBuffer);

    ensureCursorInBounds();
}

void TerminalScreen::setCursorPos(int row, int col)
{
    m_cursorRow = std::clamp(row, 0, m_rows - 1);
    m_cursorCol = std::clamp(col, 0, m_cols - 1);
}

TerminalScreen::Cell& TerminalScreen::cellAt(int row, int col)
{
    static Cell dummy;
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return dummy;
    }

    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    return buffer[row][col];
}

const TerminalScreen::Cell& TerminalScreen::cellAt(int row, int col) const
{
    static Cell dummy;
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return dummy;
    }

    const auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    return buffer[row][col];
}

void TerminalScreen::putChar(QChar ch)
{
    if (m_cursorRow >= m_rows) {
        scrollUp();
        m_cursorRow = m_rows - 1;
    }

    if (m_cursorCol >= m_cols) {
        newLine();
        return;
    }

    Cell& cell = cellAt(m_cursorRow, m_cursorCol);
    cell.character = ch;
    cell.fgColor = m_currentInverse ? m_currentBg : m_currentFg;
    cell.bgColor = m_currentInverse ? m_currentFg : m_currentBg;
    cell.bold = m_currentBold;
    cell.underline = m_currentUnderline;
    cell.inverse = m_currentInverse;
    cell.italic = m_currentItalic;

    m_cursorCol++;
}

void TerminalScreen::putChar(QChar ch, int row, int col)
{
    setCursorPos(row, col);
    putChar(ch);
}

void TerminalScreen::newLine()
{
    m_cursorRow++;
    m_cursorCol = 0;

    if (m_cursorRow > m_scrollBottom) {
        scrollUp();
        m_cursorRow = m_scrollBottom;
    }
}

void TerminalScreen::carriageReturn()
{
    m_cursorCol = 0;
}

void TerminalScreen::backspace()
{
    if (m_cursorCol > 0) {
        m_cursorCol--;
    }
}

void TerminalScreen::tab()
{
    // Move to next tab stop (every 8 columns)
    m_cursorCol = ((m_cursorCol / 8) + 1) * 8;
    if (m_cursorCol >= m_cols) {
        m_cursorCol = m_cols - 1;
    }
}

void TerminalScreen::clearScreen()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            buffer[row][col] = Cell();
        }
    }
}

void TerminalScreen::clearFromCursorToEnd()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;

    // Clear from cursor to end of line
    for (int col = m_cursorCol; col < m_cols; ++col) {
        buffer[m_cursorRow][col] = Cell();
    }

    // Clear all lines below
    for (int row = m_cursorRow + 1; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            buffer[row][col] = Cell();
        }
    }
}

void TerminalScreen::clearFromCursorToBeginning()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;

    // Clear from beginning of line to cursor
    for (int col = 0; col <= m_cursorCol; ++col) {
        buffer[m_cursorRow][col] = Cell();
    }

    // Clear all lines above
    for (int row = 0; row < m_cursorRow; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            buffer[row][col] = Cell();
        }
    }
}

void TerminalScreen::clearLine()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    for (int col = 0; col < m_cols; ++col) {
        buffer[m_cursorRow][col] = Cell();
    }
}

void TerminalScreen::clearLineFromCursor()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    for (int col = m_cursorCol; col < m_cols; ++col) {
        buffer[m_cursorRow][col] = Cell();
    }
}

void TerminalScreen::clearLineToCursor()
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;
    for (int col = 0; col <= m_cursorCol; ++col) {
        buffer[m_cursorRow][col] = Cell();
    }
}

void TerminalScreen::scrollUp(int lines)
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;

    for (int i = 0; i < lines; ++i) {
        // Move lines up within scroll region
        for (int row = m_scrollTop; row < m_scrollBottom; ++row) {
            buffer[row] = buffer[row + 1];
        }

        // Clear bottom line
        for (int col = 0; col < m_cols; ++col) {
            buffer[m_scrollBottom][col] = Cell();
        }
    }
}

void TerminalScreen::scrollDown(int lines)
{
    auto& buffer = m_useAlternate ? m_alternateBuffer : m_normalBuffer;

    for (int i = 0; i < lines; ++i) {
        // Move lines down within scroll region
        for (int row = m_scrollBottom; row > m_scrollTop; --row) {
            buffer[row] = buffer[row - 1];
        }

        // Clear top line
        for (int col = 0; col < m_cols; ++col) {
            buffer[m_scrollTop][col] = Cell();
        }
    }
}

void TerminalScreen::useAlternateBuffer()
{
    m_useAlternate = true;
    m_cursorRow = 0;
    m_cursorCol = 0;
}

void TerminalScreen::useNormalBuffer()
{
    m_useAlternate = false;
}

void TerminalScreen::resetAttributes()
{
    m_currentFg = QColor(170, 170, 170);
    m_currentBg = QColor();
    m_currentBold = false;
    m_currentUnderline = false;
    m_currentInverse = false;
    m_currentItalic = false;
}

void TerminalScreen::setScrollRegion(int top, int bottom)
{
    m_scrollTop = std::clamp(top, 0, m_rows - 1);
    m_scrollBottom = std::clamp(bottom, 0, m_rows - 1);

    if (m_scrollTop > m_scrollBottom) {
        std::swap(m_scrollTop, m_scrollBottom);
    }
}

void TerminalScreen::resetScrollRegion()
{
    m_scrollTop = 0;
    m_scrollBottom = m_rows - 1;
}

void TerminalScreen::ensureCursorInBounds()
{
    m_cursorRow = std::clamp(m_cursorRow, 0, m_rows - 1);
    m_cursorCol = std::clamp(m_cursorCol, 0, m_cols - 1);
}
