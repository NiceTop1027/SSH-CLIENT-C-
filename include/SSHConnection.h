#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include "ConnectionProfile.h"
#include <QObject>
#include <QString>
#include <libssh/libssh.h>

class SSHAuthenticator;

class SSHConnection : public QObject {
    Q_OBJECT

public:
    enum class Status { Disconnected, Connecting, Connected, Error };

    enum class ErrorType {
        None,
        InvalidHost,
        ConnectionRefused,
        AuthenticationFailed,
        NetworkError,
        Timeout,
        HostKeyVerificationFailed,
        SessionInitFailed,
        Unknown
    };

    // Constructors
    explicit SSHConnection(QObject* parent = nullptr);
    explicit SSHConnection(const ConnectionProfile& profile, QObject* parent = nullptr);
    ~SSHConnection();

    // Connection management
    void connectToHost();
    void disconnect();
    bool isConnected() const;
    Status status() const;

    // Configuration
    void setProfile(const ConnectionProfile& profile);
    ConnectionProfile profile() const;
    void setAuthenticator(SSHAuthenticator* authenticator);
    void setTimeout(int milliseconds);

    // Session management
    bool initializeSession();
    bool setSessionOptions();
    ssh_session session() const;

    // Utility methods
    QString lastError() const;
    ErrorType lastErrorType() const;
    bool waitForConnected(int timeout = 30000);

signals:
    void connected();
    void disconnected();
    void error(const QString& message);
    void authenticationFailed(const QString& message);
    void connectionTimeout();
    void statusChanged(Status status);

private slots:
    void handleConnectionResult();

private:
    void setStatus(Status status);
    void setError(const QString& message, ErrorType type = ErrorType::Unknown);
    ErrorType categorizeLibsshError(int errorCode);
    bool performConnection();
    bool verifyHostKey();
    void cleanup();

    ConnectionProfile m_profile;
    SSHAuthenticator* m_authenticator;
    ssh_session m_session;
    Status m_status;
    QString m_lastError;
    ErrorType m_lastErrorType;
    int m_timeout; // in milliseconds
};

#endif // SSHCONNECTION_H
