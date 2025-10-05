#ifndef TERMINALSCREEN_H
#define TERMINALSCREEN_H

#include <QColor>
#include <QVector>
#include <QString>

class TerminalScreen {
public:
    struct Cell {
        QChar character = ' ';
        QColor fgColor;
        QColor bgColor;
        bool bold = false;
        bool underline = false;
        bool inverse = false;
        bool italic = false;

        Cell() : character(' ') {}
    };

    TerminalScreen(int rows = 24, int cols = 80);

    // Screen dimensions
    void resize(int rows, int cols);
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    // Cursor operations
    void setCursorPos(int row, int col);
    int cursorRow() const { return m_cursorRow; }
    int cursorCol() const { return m_cursorCol; }
    bool cursorVisible() const { return m_cursorVisible; }
    void setCursorVisible(bool visible) { m_cursorVisible = visible; }

    // Cell access
    Cell& cellAt(int row, int col);
    const Cell& cellAt(int row, int col) const;

    // Text operations
    void putChar(QChar ch);
    void putChar(QChar ch, int row, int col);
    void newLine();
    void carriageReturn();
    void backspace();
    void tab();

    // Screen operations
    void clearScreen();
    void clearFromCursorToEnd();
    void clearFromCursorToBeginning();
    void clearLine();
    void clearLineFromCursor();
    void clearLineToCursor();
    void scrollUp(int lines = 1);
    void scrollDown(int lines = 1);

    // Alternate screen buffer
    void useAlternateBuffer();
    void useNormalBuffer();
    bool isAlternateBuffer() const { return m_useAlternate; }

    // Attributes
    void setFgColor(const QColor& color) { m_currentFg = color; }
    void setBgColor(const QColor& color) { m_currentBg = color; }
    void setBold(bool bold) { m_currentBold = bold; }
    void setUnderline(bool underline) { m_currentUnderline = underline; }
    void setInverse(bool inverse) { m_currentInverse = inverse; }
    void setItalic(bool italic) { m_currentItalic = italic; }
    void resetAttributes();

    QColor currentFg() const { return m_currentFg; }
    QColor currentBg() const { return m_currentBg; }
    bool currentBold() const { return m_currentBold; }

    // Scrolling region
    void setScrollRegion(int top, int bottom);
    void resetScrollRegion();

private:
    void ensureCursorInBounds();
    void initBuffer(QVector<QVector<Cell>>& buffer);

    int m_rows;
    int m_cols;
    int m_cursorRow;
    int m_cursorCol;
    bool m_cursorVisible;

    // Screen buffers
    QVector<QVector<Cell>> m_normalBuffer;
    QVector<QVector<Cell>> m_alternateBuffer;
    bool m_useAlternate;

    // Current attributes
    QColor m_currentFg;
    QColor m_currentBg;
    bool m_currentBold;
    bool m_currentUnderline;
    bool m_currentInverse;
    bool m_currentItalic;

    // Scroll region
    int m_scrollTop;
    int m_scrollBottom;
};

#endif // TERMINALSCREEN_H
