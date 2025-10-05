#ifndef TERMINALBUFFER_H
#define TERMINALBUFFER_H

#include <QString>
#include <QVector>

class TerminalBuffer {
public:
    // Constructors
    TerminalBuffer();
    TerminalBuffer(int rows, int columns);

    // Dimension management
    int rows() const;
    int columns() const;
    void setDimensions(int rows, int columns);

    // Buffer operations
    void appendLine(const QString& line);
    QString getLine(int index) const;
    QVector<QString> getVisibleLines() const;
    QVector<QString> getScrollbackBuffer() const;
    void clear();
    bool isEmpty() const;
    int lineCount() const;

    // Scrollback management
    void setMaxScrollback(int lines);
    int maxScrollback() const;

    // Cursor management
    int cursorRow() const;
    int cursorColumn() const;
    void setCursorPosition(int row, int column);

    // Text selection
    void startSelection(int row, int column);
    void updateSelection(int row, int column);
    void endSelection();
    void clearSelection();
    QString getSelectedText() const;
    bool hasSelection() const;

private:
    struct SelectionRange {
        int startRow;
        int startColumn;
        int endRow;
        int endColumn;
        bool active;

        SelectionRange() : startRow(0), startColumn(0), endRow(0), endColumn(0), active(false) {}
    };

    void enforceScrollbackLimit();
    QString wrapLine(const QString& line, int offset = 0);

    QVector<QString> m_buffer;
    int m_rows;
    int m_columns;
    int m_maxScrollback;
    int m_cursorRow;
    int m_cursorColumn;
    SelectionRange m_selection;
};

#endif // TERMINALBUFFER_H
