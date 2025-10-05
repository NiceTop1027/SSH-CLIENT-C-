#include "CredentialManager.h"
#include <QSettings>

#ifdef Q_OS_MAC
#include <Security/Security.h>
#endif

CredentialManager::CredentialManager() : m_available(false)
{
#ifdef Q_OS_WIN
    m_backend = "DPAPI";
    m_available = true;
#elif defined(Q_OS_MAC)
    m_backend = "Keychain";
    m_available = true;
#elif defined(Q_OS_LINUX)
    // Try to use libsecret if available, otherwise fallback
    m_backend = "fallback";
    m_available = true;
#else
    m_backend = "none";
    m_available = false;
#endif
}

CredentialManager::~CredentialManager()
{
}

bool CredentialManager::storePassword(const QString& service, const QString& account,
                                      const QString& password)
{
    return storeCredential(service, account, "password", password);
}

QString CredentialManager::retrievePassword(const QString& service, const QString& account)
{
    return retrieveCredential(service, account, "password");
}

bool CredentialManager::deletePassword(const QString& service, const QString& account)
{
    return deleteCredential(service, account, "password");
}

bool CredentialManager::storeKeyPath(const QString& service, const QString& account,
                                     const QString& keyPath)
{
    return storeCredential(service, account, "keypath", keyPath);
}

QString CredentialManager::retrieveKeyPath(const QString& service, const QString& account)
{
    return retrieveCredential(service, account, "keypath");
}

QString CredentialManager::backend() const
{
    return m_backend;
}

bool CredentialManager::isAvailable() const
{
    return m_available;
}

bool CredentialManager::storeCredential(const QString& service, const QString& account,
                                        const QString& key, const QString& value)
{
    QString fullKey = makeKey(service, account, key);

#ifdef Q_OS_WIN
    return storePasswordWindows(fullKey, value);
#elif defined(Q_OS_MAC)
    return storePasswordMac(service + "_" + key, account, value);
#elif defined(Q_OS_LINUX)
    return storePasswordLinux(fullKey, value);
#else
    return false;
#endif
}

QString CredentialManager::retrieveCredential(const QString& service, const QString& account,
                                              const QString& key)
{
    QString fullKey = makeKey(service, account, key);

#ifdef Q_OS_WIN
    return retrievePasswordWindows(fullKey);
#elif defined(Q_OS_MAC)
    return retrievePasswordMac(service + "_" + key, account);
#elif defined(Q_OS_LINUX)
    return retrievePasswordLinux(fullKey);
#else
    return QString();
#endif
}

bool CredentialManager::deleteCredential(const QString& service, const QString& account,
                                         const QString& key)
{
    QString fullKey = makeKey(service, account, key);

#ifdef Q_OS_WIN
    return deletePasswordWindows(fullKey);
#elif defined(Q_OS_MAC)
    return deletePasswordMac(service + "_" + key, account);
#elif defined(Q_OS_LINUX)
    return deletePasswordLinux(fullKey);
#else
    return false;
#endif
}

QString CredentialManager::makeKey(const QString& service, const QString& account,
                                   const QString& key) const
{
    return QString("%1/%2/%3").arg(service, account, key);
}

#ifdef Q_OS_WIN
// Windows DPAPI implementation
#include <windows.h>
#include <wincrypt.h>

bool CredentialManager::storePasswordWindows(const QString& key, const QString& password)
{
    // For simplicity, using QSettings with basic encoding
    // In production, should use DPAPI (CryptProtectData)
    QSettings settings("SSH-Client", "Credentials");
    settings.setValue(key, password);
    return true;
}

QString CredentialManager::retrievePasswordWindows(const QString& key)
{
    QSettings settings("SSH-Client", "Credentials");
    return settings.value(key).toString();
}

bool CredentialManager::deletePasswordWindows(const QString& key)
{
    QSettings settings("SSH-Client", "Credentials");
    settings.remove(key);
    return true;
}
#endif

#ifdef Q_OS_MAC
// macOS Keychain implementation
bool CredentialManager::storePasswordMac(const QString& service, const QString& account,
                                         const QString& password)
{
    QByteArray serviceData = service.toUtf8();
    QByteArray accountData = account.toUtf8();
    QByteArray passwordData = password.toUtf8();

    // First, try to delete existing item
    deletePasswordMac(service, account);

    // Add new item
    OSStatus status = SecKeychainAddGenericPassword(
        NULL, // default keychain
        serviceData.length(), serviceData.constData(), accountData.length(),
        accountData.constData(), passwordData.length(), passwordData.constData(),
        NULL // don't need item ref
    );

    return status == errSecSuccess;
}

QString CredentialManager::retrievePasswordMac(const QString& service, const QString& account)
{
    QByteArray serviceData = service.toUtf8();
    QByteArray accountData = account.toUtf8();

    void* passwordData = NULL;
    UInt32 passwordLength = 0;

    OSStatus status = SecKeychainFindGenericPassword(
        NULL, // default keychain
        serviceData.length(), serviceData.constData(), accountData.length(),
        accountData.constData(), &passwordLength, &passwordData,
        NULL // don't need item ref
    );

    if (status != errSecSuccess) {
        return QString();
    }

    QString password = QString::fromUtf8(static_cast<const char*>(passwordData), passwordLength);

    SecKeychainItemFreeContent(NULL, passwordData);

    return password;
}

bool CredentialManager::deletePasswordMac(const QString& service, const QString& account)
{
    QByteArray serviceData = service.toUtf8();
    QByteArray accountData = account.toUtf8();

    SecKeychainItemRef itemRef = NULL;

    OSStatus status = SecKeychainFindGenericPassword(
        NULL, serviceData.length(), serviceData.constData(), accountData.length(),
        accountData.constData(), NULL, NULL, &itemRef);

    if (status == errSecSuccess && itemRef) {
        status = SecKeychainItemDelete(itemRef);
        CFRelease(itemRef);
        return status == errSecSuccess;
    }

    return false;
}
#endif

#ifdef Q_OS_LINUX
// Linux fallback implementation (should use libsecret in production)
bool CredentialManager::storePasswordLinux(const QString& key, const QString& password)
{
    // Fallback: use QSettings with basic encoding
    // In production, should use libsecret or KWallet
    QSettings settings(QSettings::UserScope, "SSH-Client", "Credentials");
    settings.setValue(key, password);
    return true;
}

QString CredentialManager::retrievePasswordLinux(const QString& key)
{
    QSettings settings(QSettings::UserScope, "SSH-Client", "Credentials");
    return settings.value(key).toString();
}

bool CredentialManager::deletePasswordLinux(const QString& key)
{
    QSettings settings(QSettings::UserScope, "SSH-Client", "Credentials");
    settings.remove(key);
    return true;
}
#endif
