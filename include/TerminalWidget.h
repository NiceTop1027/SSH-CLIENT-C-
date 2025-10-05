#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "ANSIParser.h"
#include "TerminalBuffer.h"
#include "FileTypeColorizer.h"
#include <QWidget>
#include <QTextEdit>
#include <QFont>

class TerminalWidget : public QTextEdit {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    ~TerminalWidget();

    // Display operations
    void displayOutput(const QString& text);
    QString getDisplayedText() const;

    // Terminal dimensions
    void setDimensions(int rows, int columns);
    int rows() const;
    int columns() const;

    // Buffer operations
    void clearDisplay();
    void setMaxScrollback(int lines);

    // File coloring
    void setFileColoringEnabled(bool enabled);
    bool isFileColoringEnabled() const;

signals:
    void sendData(const QString& data);
    void dimensionsChanged(int rows, int columns);

protected:
    // Event handlers
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    struct ColorToken {
        QString text;
        QColor fgColor;
        QColor bgColor;
        bool bold = false;
    };

    void setupTerminal();
    void setupFont();
    void processANSIOutput(const QString& text);
    void applyTextFormat(const ANSIParser::TextFormat& format);
    void handleKeyInput(QKeyEvent* event);
    QString keyEventToString(QKeyEvent* event);
    void updateDisplayFromBuffer();
    QVector<ColorToken> parseAnsiWithColors(const QString& text);
    QColor getAnsiColor(int colorCode, bool bright);
    QColor get256Color(int index);

    ANSIParser m_ansiParser;
    TerminalBuffer m_buffer;
    QFont m_terminalFont;
    int m_rows;
    int m_columns;
    ANSIParser::TextFormat m_currentFormat;
    FileTypeColorizer m_fileColorizer;
    bool m_enableFileColoring;
};

#endif // TERMINALWIDGET_H
