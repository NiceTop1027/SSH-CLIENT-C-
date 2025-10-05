#include "SSHChannel.h"

SSHChannel::SSHChannel(ssh_session session, QObject* parent)
    : QObject(parent), m_session(session), m_channel(nullptr), m_isOpen(false)
{
}

SSHChannel::~SSHChannel()
{
    close();
}

bool SSHChannel::open()
{
    if (m_isOpen) {
        return true;
    }

    if (!m_session) {
        m_lastError = "Invalid SSH session";
        return false;
    }

    m_channel = ssh_channel_new(m_session);
    if (!m_channel) {
        m_lastError = "Failed to create SSH channel";
        return false;
    }

    int rc = ssh_channel_open_session(m_channel);
    if (rc != SSH_OK) {
        m_lastError = QString("Failed to open channel: %1").arg(ssh_get_error(m_session));
        ssh_channel_free(m_channel);
        m_channel = nullptr;
        return false;
    }

    m_isOpen = true;
    return true;
}

void SSHChannel::close()
{
    if (m_channel) {
        if (m_isOpen) {
            ssh_channel_close(m_channel);
            emit channelClosed();
        }
        ssh_channel_free(m_channel);
        m_channel = nullptr;
    }
    m_isOpen = false;
}

bool SSHChannel::isOpen() const
{
    return m_isOpen && m_channel != nullptr;
}

bool SSHChannel::requestShell()
{
    if (!isOpen()) {
        m_lastError = "Channel is not open";
        return false;
    }

    int rc = ssh_channel_request_shell(m_channel);
    if (rc != SSH_OK) {
        m_lastError = QString("Failed to request shell: %1").arg(ssh_get_error(m_session));
        return false;
    }

    return true;
}

bool SSHChannel::requestPty(int rows, int cols, const QString& termType)
{
    if (!isOpen()) {
        m_lastError = "Channel is not open";
        return false;
    }

    int rc = ssh_channel_request_pty_size(m_channel, termType.toUtf8().constData(), cols, rows);
    if (rc != SSH_OK) {
        m_lastError = QString("Failed to request PTY: %1").arg(ssh_get_error(m_session));
        return false;
    }

    return true;
}

int SSHChannel::write(const QString& data)
{
    return write(data.toUtf8());
}

int SSHChannel::write(const QByteArray& data)
{
    if (!isOpen()) {
        m_lastError = "Channel is not open";
        return -1;
    }

    int written = ssh_channel_write(m_channel, data.constData(), data.size());
    if (written < 0) {
        m_lastError = QString("Write failed: %1").arg(ssh_get_error(m_session));
        return -1;
    }

    return written;
}

QString SSHChannel::read(int timeout)
{
    QByteArray data = readBytes(4096, timeout);
    return QString::fromUtf8(data);
}

QByteArray SSHChannel::readBytes(int maxBytes, int timeout)
{
    if (!isOpen()) {
        m_lastError = "Channel is not open";
        return QByteArray();
    }

    QByteArray buffer(maxBytes, '\0');
    int nbytes = ssh_channel_read_timeout(m_channel, buffer.data(), maxBytes, 0, timeout);

    if (nbytes < 0) {
        m_lastError = QString("Read failed: %1").arg(ssh_get_error(m_session));
        return QByteArray();
    }

    if (nbytes == 0) {
        // No data available or EOF
        return QByteArray();
    }

    buffer.resize(nbytes);
    emit dataReceived(buffer);
    return buffer;
}

bool SSHChannel::isEof() const
{
    if (!m_channel) {
        return true;
    }
    return ssh_channel_is_eof(m_channel) != 0;
}

int SSHChannel::getExitStatus() const
{
    if (!m_channel) {
        return -1;
    }
    return ssh_channel_get_exit_status(m_channel);
}

QString SSHChannel::lastError() const
{
    return m_lastError;
}
