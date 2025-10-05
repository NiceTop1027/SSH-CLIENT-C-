#ifndef PROFILESTORAGE_H
#define PROFILESTORAGE_H

#include "ConnectionProfile.h"
#include <QString>
#include <QList>
#include <QDir>

class ProfileStorage {
public:
    // Constructor
    explicit ProfileStorage(const QString& storagePath = QString());

    // Storage management
    bool isValid() const;
    QString storagePath() const;

    // Profile operations
    bool saveProfile(const ConnectionProfile& profile);
    ConnectionProfile loadProfile(const QString& profileName);
    QList<ConnectionProfile> loadAllProfiles();
    bool deleteProfile(const QString& profileName);
    QStringList listProfiles();

    // Default storage location
    static QString defaultStoragePath();

private:
    bool ensureStorageDirectory();
    QString profileFilePath(const QString& profileName) const;
    QString sanitizeProfileName(const QString& name) const;

    QString m_storagePath;
    bool m_isValid;
};

#endif // PROFILESTORAGE_H
