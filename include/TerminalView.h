#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include "TerminalEmulator.h"
#include <QWidget>
#include <QFont>
#include <QTimer>

class TerminalView : public QWidget {
    Q_OBJECT

public:
    explicit TerminalView(QWidget* parent = nullptr);
    ~TerminalView();

    // Display operations
    void displayOutput(const QString& text);
    void displayOutput(const QByteArray& data);

    // Terminal dimensions
    void setDimensions(int rows, int columns);
    int rows() const;
    int columns() const;

    // Buffer operations
    void clearDisplay();

    // Emulator access
    TerminalEmulator& emulator() { return m_emulator; }
    const TerminalEmulator& emulator() const { return m_emulator; }

signals:
    void sendData(const QString& data);
    void dimensionsChanged(int rows, int columns);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void blinkCursor();

private:
    void setupTerminal();
    void setupFont();
    void calculateMetrics();
    QString keyEventToString(QKeyEvent* event);
    QRect getCellRect(int row, int col) const;

    TerminalEmulator m_emulator;
    QFont m_font;
    int m_charWidth;
    int m_charHeight;
    int m_rows;
    int m_columns;

    // Cursor blinking
    QTimer* m_cursorTimer;
    bool m_cursorVisible;
    bool m_hasFocus;
};

#endif // TERMINALVIEW_H
