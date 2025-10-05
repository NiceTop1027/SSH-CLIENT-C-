#include "SessionState.h"

SessionState::SessionState() : m_status(Status::Disconnected)
{
}

SessionState::SessionState(const ConnectionProfile& profile)
    : m_profile(profile), m_status(Status::Disconnected)
{
}

SessionState::Status SessionState::status() const
{
    return m_status;
}

void SessionState::setStatus(Status status)
{
    m_status = status;

    if (status == Status::Connected) {
        m_startTime = QDateTime::currentDateTime();
        m_endTime = QDateTime(); // Clear end time
    } else if (status == Status::Disconnected && m_startTime.isValid()) {
        m_endTime = QDateTime::currentDateTime();
    }
}

void SessionState::connect()
{
    setStatus(Status::Connecting);
}

void SessionState::disconnect()
{
    setStatus(Status::Disconnected);
}

void SessionState::setError(const QString& message)
{
    m_errorMessage = message;
    m_status = Status::Error;
}

QString SessionState::errorMessage() const
{
    return m_errorMessage;
}

ConnectionProfile SessionState::profile() const
{
    return m_profile;
}

QDateTime SessionState::startTime() const
{
    return m_startTime;
}

QDateTime SessionState::endTime() const
{
    return m_endTime;
}

qint64 SessionState::duration() const
{
    if (!m_startTime.isValid()) {
        return 0;
    }

    if (m_endTime.isValid()) {
        return m_startTime.msecsTo(m_endTime);
    }

    // Session is still active
    return m_startTime.msecsTo(QDateTime::currentDateTime());
}

bool SessionState::isActive() const
{
    return m_status == Status::Connecting || m_status == Status::Connected;
}
