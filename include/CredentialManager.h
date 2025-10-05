#ifndef CREDENTIALMANAGER_H
#define CREDENTIALMANAGER_H

#include <QString>

class CredentialManager {
public:
    CredentialManager();
    ~CredentialManager();

    // Password storage
    bool storePassword(const QString& service, const QString& account, const QString& password);
    QString retrievePassword(const QString& service, const QString& account);
    bool deletePassword(const QString& service, const QString& account);

    // Key path storage (for SSH keys)
    bool storeKeyPath(const QString& service, const QString& account, const QString& keyPath);
    QString retrieveKeyPath(const QString& service, const QString& account);

    // Backend info
    QString backend() const;
    bool isAvailable() const;

private:
    bool storeCredential(const QString& service, const QString& account, const QString& key,
                         const QString& value);
    QString retrieveCredential(const QString& service, const QString& account,
                               const QString& key);
    bool deleteCredential(const QString& service, const QString& account, const QString& key);

    QString makeKey(const QString& service, const QString& account, const QString& key) const;

#ifdef Q_OS_WIN
    bool storePasswordWindows(const QString& key, const QString& password);
    QString retrievePasswordWindows(const QString& key);
    bool deletePasswordWindows(const QString& key);
#elif defined(Q_OS_MAC)
    bool storePasswordMac(const QString& service, const QString& account,
                          const QString& password);
    QString retrievePasswordMac(const QString& service, const QString& account);
    bool deletePasswordMac(const QString& service, const QString& account);
#elif defined(Q_OS_LINUX)
    bool storePasswordLinux(const QString& key, const QString& password);
    QString retrievePasswordLinux(const QString& key);
    bool deletePasswordLinux(const QString& key);
#endif

    QString m_backend;
    bool m_available;
};

#endif // CREDENTIALMANAGER_H
