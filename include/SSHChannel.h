#ifndef SSHCHANNEL_H
#define SSHCHANNEL_H

#include <QObject>
#include <QString>
#include <libssh/libssh.h>

class SSHChannel : public QObject {
    Q_OBJECT

public:
    // Constructor
    explicit SSHChannel(ssh_session session, QObject* parent = nullptr);
    ~SSHChannel();

    // Channel lifecycle
    bool open();
    void close();
    bool isOpen() const;

    // Shell operations
    bool requestShell();
    bool requestPty(int rows = 24, int cols = 80, const QString& termType = "xterm-256color");

    // I/O operations
    int write(const QString& data);
    int write(const QByteArray& data);
    QString read(int timeout = 1000);
    QByteArray readBytes(int maxBytes = 4096, int timeout = 1000);

    // Channel state
    bool isEof() const;
    int getExitStatus() const;

    // Utility
    QString lastError() const;

signals:
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);
    void channelClosed();

private:
    ssh_session m_session;
    ssh_channel m_channel;
    QString m_lastError;
    bool m_isOpen;
};

#endif // SSHCHANNEL_H
