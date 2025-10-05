#ifndef ANSIPARSER_H
#define ANSIPARSER_H

#include <QString>
#include <QVector>
#include <QColor>

class ANSIParser {
public:
    enum class CommandType {
        None,
        CursorUp,
        CursorDown,
        CursorForward,
        CursorBackward,
        CursorPosition,
        EraseDisplay,
        EraseLine,
        SetGraphicsMode
    };

    enum class Color {
        Default,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightBlack,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite
    };

    struct TextFormat {
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool inverse = false;
        Color foregroundColor = Color::Default;
        Color backgroundColor = Color::Default;

        void reset() {
            bold = false;
            italic = false;
            underline = false;
            inverse = false;
            foregroundColor = Color::Default;
            backgroundColor = Color::Default;
        }
    };

    struct Command {
        CommandType type;
        int param1 = 0;
        int param2 = 0;
        QVector<int> params;
    };

    struct Token {
        QString text;
        TextFormat format;
    };

    // Constructor
    ANSIParser();

    // Parsing methods
    QString parse(const QString& input);
    QVector<Token> parseToTokens(const QString& input);
    Command parseCommand(const QString& sequence);

    // State queries
    bool hasEscapeSequences() const;
    TextFormat currentFormat() const;

    // Color conversion
    static QColor toQColor(Color color);

private:
    struct ParseState {
        int position = 0;
        TextFormat format;
        QString currentText;
        QVector<Token> tokens;
    };

    void processEscapeSequence(const QString& input, ParseState& state);
    void processSGR(const QVector<int>& params, ParseState& state);
    void applyFormatChange(int param, ParseState& state);
    QVector<int> extractParameters(const QString& sequence);
    bool isEscapeSequenceStart(const QString& input, int pos) const;
    int findSequenceEnd(const QString& input, int start) const;

    TextFormat m_currentFormat;
    bool m_hasEscapeSequences;
};

#endif // ANSIPARSER_H
