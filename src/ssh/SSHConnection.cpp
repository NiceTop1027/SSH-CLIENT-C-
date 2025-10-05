#include "SSHConnection.h"
#include "SSHAuthenticator.h"
#include "Logger.h"
#include <QTimer>
#include <QCoreApplication>

SSHConnection::SSHConnection(QObject* parent)
    : QObject(parent), m_authenticator(nullptr), m_session(nullptr),
      m_status(Status::Disconnected), m_lastErrorType(ErrorType::None), m_timeout(30000)
{
    qInfo(sshConnection) << "SSHConnection created";
}

SSHConnection::SSHConnection(const ConnectionProfile& profile, QObject* parent)
    : QObject(parent), m_profile(profile), m_authenticator(nullptr), m_session(nullptr),
      m_status(Status::Disconnected), m_lastErrorType(ErrorType::None), m_timeout(30000)
{
    qInfo(sshConnection) << "SSHConnection created for host:" << profile.hostname();
}

SSHConnection::~SSHConnection()
{
    cleanup();
}

void SSHConnection::connectToHost()
{
    if (m_status == Status::Connected || m_status == Status::Connecting) {
        qWarning(sshConnection) << "Already connected or connecting to" << m_profile.hostname();
        return; // Already connected or connecting
    }

    qInfo(sshConnection) << "Initiating connection to" << m_profile.hostname() << ":"
                         << m_profile.port() << "as" << m_profile.username();
    setStatus(Status::Connecting);

    if (!initializeSession()) {
        setError("Failed to initialize SSH session", ErrorType::SessionInitFailed);
        return;
    }

    if (!setSessionOptions()) {
        setError("Failed to set session options", ErrorType::SessionInitFailed);
        cleanup();
        return;
    }

    // Connect in the main thread (could be moved to worker thread later)
    QTimer::singleShot(0, this, &SSHConnection::handleConnectionResult);
}

void SSHConnection::disconnect()
{
    if (m_status == Status::Disconnected) {
        qDebug(sshConnection) << "Already disconnected";
        return;
    }

    qInfo(sshConnection) << "Disconnecting from" << m_profile.hostname();
    cleanup();
    setStatus(Status::Disconnected);
    emit disconnected();
}

bool SSHConnection::isConnected() const
{
    return m_status == Status::Connected;
}

SSHConnection::Status SSHConnection::status() const
{
    return m_status;
}

void SSHConnection::setProfile(const ConnectionProfile& profile)
{
    m_profile = profile;
}

ConnectionProfile SSHConnection::profile() const
{
    return m_profile;
}

void SSHConnection::setAuthenticator(SSHAuthenticator* authenticator)
{
    m_authenticator = authenticator;
}

void SSHConnection::setTimeout(int milliseconds)
{
    m_timeout = milliseconds;
}

bool SSHConnection::initializeSession()
{
    m_session = ssh_new();
    if (!m_session) {
        return false;
    }
    return true;
}

bool SSHConnection::setSessionOptions()
{
    if (!m_session) {
        return false;
    }

    // Set hostname
    if (ssh_options_set(m_session, SSH_OPTIONS_HOST, m_profile.hostname().toUtf8().constData()) <
        0) {
        return false;
    }

    // Set port
    unsigned int port = m_profile.port();
    if (ssh_options_set(m_session, SSH_OPTIONS_PORT, &port) < 0) {
        return false;
    }

    // Set username
    if (ssh_options_set(m_session, SSH_OPTIONS_USER, m_profile.username().toUtf8().constData()) <
        0) {
        return false;
    }

    // Set timeout (convert milliseconds to seconds)
    long timeout_sec = m_timeout / 1000;
    ssh_options_set(m_session, SSH_OPTIONS_TIMEOUT, &timeout_sec);

    return true;
}

ssh_session SSHConnection::session() const
{
    return m_session;
}

QString SSHConnection::lastError() const
{
    return m_lastError;
}

SSHConnection::ErrorType SSHConnection::lastErrorType() const
{
    return m_lastErrorType;
}

bool SSHConnection::waitForConnected(int timeout)
{
    if (isConnected()) {
        return true;
    }

    // Simple blocking wait (should be improved with event loop)
    QTimer timer;
    timer.setSingleShot(true);
    bool timedOut = false;

    connect(&timer, &QTimer::timeout, [&timedOut]() { timedOut = true; });

    timer.start(timeout);

    while (!isConnected() && !timedOut && m_status != Status::Error) {
        QCoreApplication::processEvents();
    }

    return isConnected();
}

void SSHConnection::handleConnectionResult()
{
    if (!performConnection()) {
        return; // Error already set
    }

    if (!verifyHostKey()) {
        setError("Host key verification failed", ErrorType::HostKeyVerificationFailed);
        cleanup();
        return;
    }

    // Authenticate
    if (m_authenticator && !m_authenticator->authenticate(m_session)) {
        QString authError = m_authenticator->lastError();
        setError("Authentication failed: " + authError, ErrorType::AuthenticationFailed);
        qCritical(sshAuth) << "Authentication failed for" << m_profile.username() << "@"
                           << m_profile.hostname() << ":" << authError;
        emit authenticationFailed(m_lastError);
        cleanup();
        return;
    } else if (!m_authenticator) {
        setError("No authenticator provided", ErrorType::AuthenticationFailed);
        cleanup();
        return;
    }

    qInfo(sshConnection) << "Successfully connected to" << m_profile.hostname();
    setStatus(Status::Connected);
    emit connected();
}

void SSHConnection::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void SSHConnection::setError(const QString& message, ErrorType type)
{
    m_lastError = message;
    m_lastErrorType = type;
    qCritical(sshConnection) << "SSH Error:" << message << "Type:" << static_cast<int>(type);
    setStatus(Status::Error);
    emit error(message);
}

SSHConnection::ErrorType SSHConnection::categorizeLibsshError(int errorCode)
{
    // Map libssh errors to our error types
    // Note: libssh doesn't provide detailed error codes, so we parse error message
    if (!m_session) {
        return ErrorType::SessionInitFailed;
    }

    const char* errorMsg = ssh_get_error(m_session);
    QString errorStr = QString::fromUtf8(errorMsg).toLower();

    if (errorStr.contains("timeout") || errorStr.contains("timed out")) {
        return ErrorType::Timeout;
    } else if (errorStr.contains("refused") || errorStr.contains("connection refused")) {
        return ErrorType::ConnectionRefused;
    } else if (errorStr.contains("host") || errorStr.contains("resolve")) {
        return ErrorType::InvalidHost;
    } else if (errorStr.contains("network") || errorStr.contains("unreachable")) {
        return ErrorType::NetworkError;
    } else if (errorStr.contains("auth")) {
        return ErrorType::AuthenticationFailed;
    }

    return ErrorType::Unknown;
}

bool SSHConnection::performConnection()
{
    if (!m_session) {
        setError("Session not initialized", ErrorType::SessionInitFailed);
        return false;
    }

    qDebug(sshConnection) << "Performing SSH connection to" << m_profile.hostname();

    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        QString error = QString("Connection failed: %1").arg(ssh_get_error(m_session));
        ErrorType errorType = categorizeLibsshError(rc);
        setError(error, errorType);
        return false;
    }

    qDebug(sshConnection) << "TCP connection established";
    return true;
}

bool SSHConnection::verifyHostKey()
{
    if (!m_session) {
        return false;
    }

    // Get the public key from the server
    ssh_key srv_pubkey = nullptr;
    int rc = ssh_get_server_publickey(m_session, &srv_pubkey);
    if (rc < 0) {
        return false;
    }

    // For now, accept all host keys (INSECURE - should be improved)
    // In production, should check known_hosts file
    ssh_key_free(srv_pubkey);

    // TODO: Implement proper host key verification
    // - Check against known_hosts
    // - Prompt user for unknown hosts
    // - Store accepted keys

    return true;
}

void SSHConnection::cleanup()
{
    if (m_session) {
        if (m_status == Status::Connected) {
            qDebug(sshConnection) << "Disconnecting SSH session";
            ssh_disconnect(m_session);
        }
        qDebug(sshConnection) << "Freeing SSH session";
        ssh_free(m_session);
        m_session = nullptr;
    }
    m_lastErrorType = ErrorType::None;
}
