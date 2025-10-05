#include "SSHAuthenticator.h"
#include <QFile>
#include <QFileInfo>

SSHAuthenticator::SSHAuthenticator() : m_authMethod(AuthMethod::None)
{
}

SSHAuthenticator::SSHAuthenticator(const ConnectionProfile& profile, const QString& password, const QString& keyPath)
    : m_authMethod(AuthMethod::None)
{
    if (!password.isEmpty()) {
        setPassword(password);
    } else if (!keyPath.isEmpty()) {
        setPrivateKeyFile(keyPath);
    } else if (profile.authMethod() == ConnectionProfile::AuthMethod::Password) {
        m_authMethod = AuthMethod::Password;
    } else if (profile.authMethod() == ConnectionProfile::AuthMethod::PublicKey) {
        if (!profile.keyFilePath().isEmpty()) {
            setPrivateKeyFile(profile.keyFilePath());
        }
    }
}

SSHAuthenticator::~SSHAuthenticator()
{
    // Securely clear password from memory
    m_password.fill('0');
    m_keyPassphrase.fill('0');
}

void SSHAuthenticator::setPassword(const QString& password)
{
    if (password.isEmpty()) {
        m_authMethod = AuthMethod::None;
        m_password.clear();
    } else {
        m_authMethod = AuthMethod::Password;
        m_password = password;
    }
}

void SSHAuthenticator::setPrivateKeyFile(const QString& keyPath)
{
    m_keyFilePath = keyPath;
    m_authMethod = AuthMethod::PublicKey;
}

void SSHAuthenticator::setKeyPassphrase(const QString& passphrase)
{
    m_keyPassphrase = passphrase;
}

void SSHAuthenticator::clearCredentials()
{
    m_password.fill('0');
    m_password.clear();
    m_keyPassphrase.fill('0');
    m_keyPassphrase.clear();
    m_keyFilePath.clear();
    m_authMethod = AuthMethod::None;
}

bool SSHAuthenticator::authenticate(ssh_session session)
{
    if (!session) {
        m_lastError = "Invalid SSH session";
        return false;
    }

    if (!hasCredentials()) {
        m_lastError = "No credentials provided";
        return false;
    }

    // Try authentication directly with the chosen method
    bool result = false;

    switch (m_authMethod) {
    case AuthMethod::Password:
        result = authenticateWithPassword(session);
        break;

    case AuthMethod::PublicKey:
        result = authenticateWithPublicKey(session);
        break;

    case AuthMethod::None:
    default:
        m_lastError = "No authentication method set";
        return false;
    }

    return result;
}

bool SSHAuthenticator::hasCredentials() const
{
    switch (m_authMethod) {
    case AuthMethod::Password:
        return !m_password.isEmpty();

    case AuthMethod::PublicKey:
        return !m_keyFilePath.isEmpty();

    case AuthMethod::None:
    default:
        return false;
    }
}

SSHAuthenticator::AuthMethod SSHAuthenticator::authMethod() const
{
    return m_authMethod;
}

bool SSHAuthenticator::isKeyFileValid() const
{
    if (m_keyFilePath.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(m_keyFilePath);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable();
}

bool SSHAuthenticator::validateKeyFile()
{
    if (!isKeyFileValid()) {
        m_lastError = "Key file not found or not readable";
        return false;
    }

    // TODO: Additional validation (check if it's a valid SSH key format)
    return true;
}

QString SSHAuthenticator::lastError() const
{
    return m_lastError;
}

bool SSHAuthenticator::authenticateWithPassword(ssh_session session)
{
    if (m_password.isEmpty()) {
        m_lastError = "Password is empty";
        return false;
    }

    int rc = ssh_userauth_password(session, nullptr, m_password.toUtf8().constData());

    if (rc == SSH_AUTH_SUCCESS) {
        return true;
    }

    m_lastError = QString("Password authentication failed: %1").arg(ssh_get_error(session));
    return false;
}

bool SSHAuthenticator::authenticateWithPublicKey(ssh_session session)
{
    if (!isKeyFileValid()) {
        m_lastError = "Invalid or missing private key file";
        return false;
    }

    // Import private key
    ssh_key privkey = nullptr;
    int rc;

    if (!m_keyPassphrase.isEmpty()) {
        rc = ssh_pki_import_privkey_file(m_keyFilePath.toUtf8().constData(),
                                         m_keyPassphrase.toUtf8().constData(), nullptr, nullptr,
                                         &privkey);
    } else {
        rc = ssh_pki_import_privkey_file(m_keyFilePath.toUtf8().constData(), nullptr, nullptr,
                                         nullptr, &privkey);
    }

    if (rc != SSH_OK) {
        m_lastError = QString("Failed to import private key: %1").arg(ssh_get_error(session));
        return false;
    }

    // Try to authenticate with the key
    rc = ssh_userauth_publickey(session, nullptr, privkey);

    ssh_key_free(privkey);

    if (rc == SSH_AUTH_SUCCESS) {
        return true;
    }

    m_lastError =
        QString("Public key authentication failed: %1").arg(ssh_get_error(session));
    return false;
}
