#include "ANSIParser.h"

ANSIParser::ANSIParser() : m_hasEscapeSequences(false)
{
}

QString ANSIParser::parse(const QString& input)
{
    if (input.isEmpty()) {
        return QString();
    }

    ParseState state;
    m_hasEscapeSequences = false;

    while (state.position < input.length()) {
        if (isEscapeSequenceStart(input, state.position)) {
            m_hasEscapeSequences = true;
            processEscapeSequence(input, state);
        } else {
            state.currentText += input[state.position];
            state.position++;
        }
    }

    // Return just the text without formatting
    QString result;
    for (const auto& token : state.tokens) {
        result += token.text;
    }
    result += state.currentText;

    return result;
}

QVector<ANSIParser::Token> ANSIParser::parseToTokens(const QString& input)
{
    if (input.isEmpty()) {
        return QVector<Token>();
    }

    ParseState state;
    m_hasEscapeSequences = false;

    while (state.position < input.length()) {
        if (isEscapeSequenceStart(input, state.position)) {
            m_hasEscapeSequences = true;

            // Save current text with current format
            if (!state.currentText.isEmpty()) {
                Token token;
                token.text = state.currentText;
                token.format = state.format;
                state.tokens.append(token);
                state.currentText.clear();
            }

            processEscapeSequence(input, state);
        } else {
            state.currentText += input[state.position];
            state.position++;
        }
    }

    // Add remaining text
    if (!state.currentText.isEmpty()) {
        Token token;
        token.text = state.currentText;
        token.format = state.format;
        state.tokens.append(token);
    }

    m_currentFormat = state.format;
    return state.tokens;
}

ANSIParser::Command ANSIParser::parseCommand(const QString& sequence)
{
    Command cmd;
    cmd.type = CommandType::None;

    if (sequence.isEmpty() || !sequence.startsWith("\x1b[")) {
        return cmd;
    }

    // Extract the command character (last char)
    QChar commandChar = sequence[sequence.length() - 1];

    // Extract parameters
    cmd.params = extractParameters(sequence);

    // Parse command type based on final character
    switch (commandChar.toLatin1()) {
    case 'A':
        cmd.type = CommandType::CursorUp;
        cmd.param1 = cmd.params.isEmpty() ? 1 : cmd.params[0];
        break;
    case 'B':
        cmd.type = CommandType::CursorDown;
        cmd.param1 = cmd.params.isEmpty() ? 1 : cmd.params[0];
        break;
    case 'C':
        cmd.type = CommandType::CursorForward;
        cmd.param1 = cmd.params.isEmpty() ? 1 : cmd.params[0];
        break;
    case 'D':
        cmd.type = CommandType::CursorBackward;
        cmd.param1 = cmd.params.isEmpty() ? 1 : cmd.params[0];
        break;
    case 'H':
    case 'f':
        cmd.type = CommandType::CursorPosition;
        cmd.param1 = cmd.params.isEmpty() ? 1 : cmd.params[0];
        cmd.param2 = cmd.params.size() < 2 ? 1 : cmd.params[1];
        break;
    case 'J':
        cmd.type = CommandType::EraseDisplay;
        cmd.param1 = cmd.params.isEmpty() ? 0 : cmd.params[0];
        break;
    case 'K':
        cmd.type = CommandType::EraseLine;
        cmd.param1 = cmd.params.isEmpty() ? 0 : cmd.params[0];
        break;
    case 'm':
        cmd.type = CommandType::SetGraphicsMode;
        break;
    default:
        cmd.type = CommandType::None;
        break;
    }

    return cmd;
}

bool ANSIParser::hasEscapeSequences() const
{
    return m_hasEscapeSequences;
}

ANSIParser::TextFormat ANSIParser::currentFormat() const
{
    return m_currentFormat;
}

QColor ANSIParser::toQColor(Color color)
{
    switch (color) {
    case Color::Black:
        return QColor(0, 0, 0);
    case Color::Red:
        return QColor(170, 0, 0);
    case Color::Green:
        return QColor(0, 170, 0);
    case Color::Yellow:
        return QColor(170, 85, 0);
    case Color::Blue:
        return QColor(0, 0, 170);
    case Color::Magenta:
        return QColor(170, 0, 170);
    case Color::Cyan:
        return QColor(0, 170, 170);
    case Color::White:
        return QColor(170, 170, 170);
    case Color::BrightBlack:
        return QColor(85, 85, 85);
    case Color::BrightRed:
        return QColor(255, 85, 85);
    case Color::BrightGreen:
        return QColor(85, 255, 85);
    case Color::BrightYellow:
        return QColor(255, 255, 85);
    case Color::BrightBlue:
        return QColor(85, 85, 255);
    case Color::BrightMagenta:
        return QColor(255, 85, 255);
    case Color::BrightCyan:
        return QColor(85, 255, 255);
    case Color::BrightWhite:
        return QColor(255, 255, 255);
    case Color::Default:
    default:
        return QColor();
    }
}

void ANSIParser::processEscapeSequence(const QString& input, ParseState& state)
{
    int seqEnd = findSequenceEnd(input, state.position);

    if (seqEnd == -1) {
        // Incomplete sequence, skip ESC character
        state.position++;
        return;
    }

    QString sequence = input.mid(state.position, seqEnd - state.position + 1);
    Command cmd = parseCommand(sequence);

    // Handle SGR (text formatting) commands
    if (cmd.type == CommandType::SetGraphicsMode) {
        processSGR(cmd.params, state);
    }
    // Other commands (cursor movement, erase) would be handled by terminal widget

    state.position = seqEnd + 1;
}

void ANSIParser::processSGR(const QVector<int>& params, ParseState& state)
{
    if (params.isEmpty()) {
        state.format.reset();
        return;
    }

    for (int param : params) {
        applyFormatChange(param, state);
    }
}

void ANSIParser::applyFormatChange(int param, ParseState& state)
{
    switch (param) {
    case 0: // Reset
        state.format.reset();
        break;
    case 1: // Bold
        state.format.bold = true;
        break;
    case 3: // Italic
        state.format.italic = true;
        break;
    case 4: // Underline
        state.format.underline = true;
        break;
    case 7: // Inverse
        state.format.inverse = true;
        break;
    case 22: // Normal intensity
        state.format.bold = false;
        break;
    case 23: // Not italic
        state.format.italic = false;
        break;
    case 24: // Not underlined
        state.format.underline = false;
        break;
    case 27: // Not inverse
        state.format.inverse = false;
        break;

    // Foreground colors (30-37)
    case 30:
        state.format.foregroundColor = Color::Black;
        break;
    case 31:
        state.format.foregroundColor = Color::Red;
        break;
    case 32:
        state.format.foregroundColor = Color::Green;
        break;
    case 33:
        state.format.foregroundColor = Color::Yellow;
        break;
    case 34:
        state.format.foregroundColor = Color::Blue;
        break;
    case 35:
        state.format.foregroundColor = Color::Magenta;
        break;
    case 36:
        state.format.foregroundColor = Color::Cyan;
        break;
    case 37:
        state.format.foregroundColor = Color::White;
        break;
    case 39: // Default foreground
        state.format.foregroundColor = Color::Default;
        break;

    // Background colors (40-47)
    case 40:
        state.format.backgroundColor = Color::Black;
        break;
    case 41:
        state.format.backgroundColor = Color::Red;
        break;
    case 42:
        state.format.backgroundColor = Color::Green;
        break;
    case 43:
        state.format.backgroundColor = Color::Yellow;
        break;
    case 44:
        state.format.backgroundColor = Color::Blue;
        break;
    case 45:
        state.format.backgroundColor = Color::Magenta;
        break;
    case 46:
        state.format.backgroundColor = Color::Cyan;
        break;
    case 47:
        state.format.backgroundColor = Color::White;
        break;
    case 49: // Default background
        state.format.backgroundColor = Color::Default;
        break;

    // Bright foreground colors (90-97)
    case 90:
        state.format.foregroundColor = Color::BrightBlack;
        break;
    case 91:
        state.format.foregroundColor = Color::BrightRed;
        break;
    case 92:
        state.format.foregroundColor = Color::BrightGreen;
        break;
    case 93:
        state.format.foregroundColor = Color::BrightYellow;
        break;
    case 94:
        state.format.foregroundColor = Color::BrightBlue;
        break;
    case 95:
        state.format.foregroundColor = Color::BrightMagenta;
        break;
    case 96:
        state.format.foregroundColor = Color::BrightCyan;
        break;
    case 97:
        state.format.foregroundColor = Color::BrightWhite;
        break;

    // Bright background colors (100-107)
    case 100:
        state.format.backgroundColor = Color::BrightBlack;
        break;
    case 101:
        state.format.backgroundColor = Color::BrightRed;
        break;
    case 102:
        state.format.backgroundColor = Color::BrightGreen;
        break;
    case 103:
        state.format.backgroundColor = Color::BrightYellow;
        break;
    case 104:
        state.format.backgroundColor = Color::BrightBlue;
        break;
    case 105:
        state.format.backgroundColor = Color::BrightMagenta;
        break;
    case 106:
        state.format.backgroundColor = Color::BrightCyan;
        break;
    case 107:
        state.format.backgroundColor = Color::BrightWhite;
        break;

    default:
        // Unknown parameter, ignore
        break;
    }
}

QVector<int> ANSIParser::extractParameters(const QString& sequence)
{
    QVector<int> params;

    // Extract content between ESC[ and final character
    if (sequence.length() < 3 || !sequence.startsWith("\x1b[")) {
        return params;
    }

    QString paramStr = sequence.mid(2, sequence.length() - 3);

    if (paramStr.isEmpty()) {
        return params;
    }

    QStringList parts = paramStr.split(';');
    for (const QString& part : parts) {
        bool ok;
        int value = part.toInt(&ok);
        if (ok) {
            params.append(value);
        }
    }

    return params;
}

bool ANSIParser::isEscapeSequenceStart(const QString& input, int pos) const
{
    if (pos >= input.length()) {
        return false;
    }

    // Check for ESC character followed by [
    if (input[pos] == '\x1b' && pos + 1 < input.length() && input[pos + 1] == '[') {
        return true;
    }

    return false;
}

int ANSIParser::findSequenceEnd(const QString& input, int start) const
{
    if (start + 1 >= input.length()) {
        return -1;
    }

    // Start after ESC[
    int pos = start + 2;

    // Find the final character (letter)
    while (pos < input.length()) {
        QChar c = input[pos];

        // Final character is a letter (A-Z, a-z)
        if (c.isLetter()) {
            return pos;
        }

        // Valid intermediate characters: digits, semicolon, space
        if (!c.isDigit() && c != ';' && c != ' ') {
            return -1; // Invalid sequence
        }

        pos++;
    }

    return -1; // Incomplete sequence
}
