#ifndef SSHAUTHENTICATOR_H
#define SSHAUTHENTICATOR_H

#include "ConnectionProfile.h"
#include <QString>
#include <libssh/libssh.h>

class SSHAuthenticator {
public:
    enum class AuthMethod { None, Password, PublicKey };

    // Constructors
    SSHAuthenticator();
    SSHAuthenticator(const ConnectionProfile& profile, const QString& password = QString(), const QString& keyPath = QString());
    ~SSHAuthenticator();

    // Credential management
    void setPassword(const QString& password);
    void setPrivateKeyFile(const QString& keyPath);
    void setKeyPassphrase(const QString& passphrase);
    void clearCredentials();

    // Authentication
    bool authenticate(ssh_session session);
    bool hasCredentials() const;
    AuthMethod authMethod() const;

    // Key validation
    bool isKeyFileValid() const;
    bool validateKeyFile();

    // Utility
    QString lastError() const;

private:
    bool authenticateWithPassword(ssh_session session);
    bool authenticateWithPublicKey(ssh_session session);

    QString m_password;
    QString m_keyFilePath;
    QString m_keyPassphrase;
    AuthMethod m_authMethod;
    QString m_lastError;
};

#endif // SSHAUTHENTICATOR_H
