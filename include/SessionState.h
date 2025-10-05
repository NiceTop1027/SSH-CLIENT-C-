#ifndef SESSIONSTATE_H
#define SESSIONSTATE_H

#include "ConnectionProfile.h"
#include <QDateTime>
#include <QString>

class SessionState {
public:
    enum class Status { Disconnected, Connecting, Connected, Error };

    // Constructors
    SessionState();
    explicit SessionState(const ConnectionProfile& profile);

    // Status management
    Status status() const;
    void setStatus(Status status);

    // Connection lifecycle
    void connect();
    void disconnect();

    // Error handling
    void setError(const QString& message);
    QString errorMessage() const;

    // Session information
    ConnectionProfile profile() const;
    QDateTime startTime() const;
    QDateTime endTime() const;
    qint64 duration() const; // Duration in milliseconds
    bool isActive() const;

private:
    ConnectionProfile m_profile;
    Status m_status;
    QString m_errorMessage;
    QDateTime m_startTime;
    QDateTime m_endTime;
};

#endif // SESSIONSTATE_H
