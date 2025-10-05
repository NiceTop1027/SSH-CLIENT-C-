#include "TerminalBuffer.h"
#include <algorithm>

TerminalBuffer::TerminalBuffer()
    : m_rows(24), m_columns(80), m_maxScrollback(10000), m_cursorRow(0), m_cursorColumn(0)
{
}

TerminalBuffer::TerminalBuffer(int rows, int columns)
    : m_rows(rows), m_columns(columns), m_maxScrollback(10000), m_cursorRow(0), m_cursorColumn(0)
{
}

int TerminalBuffer::rows() const
{
    return m_rows;
}

int TerminalBuffer::columns() const
{
    return m_columns;
}

void TerminalBuffer::setDimensions(int rows, int columns)
{
    m_rows = rows;
    m_columns = columns;

    // Clamp cursor to new bounds
    if (m_cursorRow >= rows) {
        m_cursorRow = rows - 1;
    }
    if (m_cursorColumn >= columns) {
        m_cursorColumn = columns - 1;
    }
}

void TerminalBuffer::appendLine(const QString& line)
{
    if (line.length() <= m_columns) {
        m_buffer.append(line);
    } else {
        // Handle line wrapping
        int offset = 0;
        while (offset < line.length()) {
            m_buffer.append(wrapLine(line, offset));
            offset += m_columns;
        }
    }

    enforceScrollbackLimit();
}

QString TerminalBuffer::getLine(int index) const
{
    if (index >= 0 && index < m_buffer.size()) {
        return m_buffer[index];
    }
    return QString();
}

QVector<QString> TerminalBuffer::getVisibleLines() const
{
    QVector<QString> visible;
    int start = std::max(0, static_cast<int>(m_buffer.size()) - m_rows);

    for (int i = start; i < m_buffer.size(); i++) {
        visible.append(m_buffer[i]);
    }

    // Pad with empty lines if needed
    while (visible.size() < m_rows) {
        visible.append(QString());
    }

    return visible;
}

QVector<QString> TerminalBuffer::getScrollbackBuffer() const
{
    return m_buffer;
}

void TerminalBuffer::clear()
{
    m_buffer.clear();
    m_cursorRow = 0;
    m_cursorColumn = 0;
    m_selection = SelectionRange();
}

bool TerminalBuffer::isEmpty() const
{
    return m_buffer.isEmpty();
}

int TerminalBuffer::lineCount() const
{
    return m_buffer.size();
}

void TerminalBuffer::setMaxScrollback(int lines)
{
    m_maxScrollback = lines;
    enforceScrollbackLimit();
}

int TerminalBuffer::maxScrollback() const
{
    return m_maxScrollback;
}

int TerminalBuffer::cursorRow() const
{
    return m_cursorRow;
}

int TerminalBuffer::cursorColumn() const
{
    return m_cursorColumn;
}

void TerminalBuffer::setCursorPosition(int row, int column)
{
    m_cursorRow = std::clamp(row, 0, m_rows - 1);
    m_cursorColumn = std::clamp(column, 0, m_columns - 1);
}

void TerminalBuffer::startSelection(int row, int column)
{
    m_selection.active = true;
    m_selection.startRow = std::clamp(row, 0, static_cast<int>(m_buffer.size()) - 1);
    m_selection.startColumn = std::clamp(column, 0, m_columns - 1);
    m_selection.endRow = m_selection.startRow;
    m_selection.endColumn = m_selection.startColumn;
}

void TerminalBuffer::updateSelection(int row, int column)
{
    if (m_selection.active) {
        m_selection.endRow = std::clamp(row, 0, static_cast<int>(m_buffer.size()) - 1);
        m_selection.endColumn = std::clamp(column, 0, m_columns - 1);
    }
}

void TerminalBuffer::endSelection()
{
    // Selection is finalized, remains active until cleared
}

void TerminalBuffer::clearSelection()
{
    m_selection = SelectionRange();
}

QString TerminalBuffer::getSelectedText() const
{
    if (!m_selection.active) {
        return QString();
    }

    int startRow = std::min(m_selection.startRow, m_selection.endRow);
    int endRow = std::max(m_selection.startRow, m_selection.endRow);
    int startCol = (m_selection.startRow <= m_selection.endRow) ? m_selection.startColumn
                                                                 : m_selection.endColumn;
    int endCol = (m_selection.startRow <= m_selection.endRow) ? m_selection.endColumn
                                                               : m_selection.startColumn;

    QString result;

    if (startRow == endRow) {
        // Single line selection
        QString line = getLine(startRow);
        result = line.mid(startCol, endCol - startCol);
    } else {
        // Multi-line selection
        for (int row = startRow; row <= endRow; row++) {
            QString line = getLine(row);

            if (row == startRow) {
                result += line.mid(startCol);
            } else if (row == endRow) {
                result += line.left(endCol);
            } else {
                result += line;
            }

            if (row < endRow) {
                result += "\n";
            }
        }
    }

    return result;
}

bool TerminalBuffer::hasSelection() const
{
    return m_selection.active;
}

void TerminalBuffer::enforceScrollbackLimit()
{
    while (m_buffer.size() > m_maxScrollback) {
        m_buffer.removeFirst();
    }
}

QString TerminalBuffer::wrapLine(const QString& line, int offset)
{
    return line.mid(offset, m_columns);
}
