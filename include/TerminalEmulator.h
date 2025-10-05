#ifndef TERMINALEMULATOR_H
#define TERMINALEMULATOR_H

#include "TerminalScreen.h"
#include <QString>
#include <QQueue>

class TerminalEmulator {
public:
    TerminalEmulator(int rows = 24, int cols = 80);

    // Process incoming data
    void processData(const QString& data);
    void processData(const QByteArray& data);

    // Screen access
    TerminalScreen& screen() { return m_screen; }
    const TerminalScreen& screen() const { return m_screen; }

    // Dimensions
    void resize(int rows, int cols);
    int rows() const { return m_screen.rows(); }
    int cols() const { return m_screen.cols(); }

private:
    void processChar(QChar ch);
    void handleEscapeSequence();
    void handleCSI();
    void handleOSC();

    void executeCsiCommand(QChar command, const QVector<int>& params);
    QColor getAnsiColor(int colorCode, bool bright = false) const;
    QColor get256Color(int index) const;

    TerminalScreen m_screen;

    enum class State {
        Normal,
        Escape,
        CSI,
        OSC,
        OSC_String
    };

    State m_state;
    QString m_escapeBuffer;
    QString m_oscString;
};

#endif // TERMINALEMULATOR_H
