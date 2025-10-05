#include "ProfileStorage.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDirIterator>

ProfileStorage::ProfileStorage(const QString& storagePath) : m_isValid(false)
{
    if (storagePath.isEmpty()) {
        m_storagePath = defaultStoragePath();
    } else {
        m_storagePath = storagePath;
    }

    m_isValid = ensureStorageDirectory();
}

bool ProfileStorage::isValid() const
{
    return m_isValid;
}

QString ProfileStorage::storagePath() const
{
    return m_storagePath;
}

bool ProfileStorage::saveProfile(const ConnectionProfile& profile)
{
    if (!m_isValid || profile.profileName().isEmpty()) {
        return false;
    }

    QString filePath = profileFilePath(profile.profileName());
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject json = profile.toJson();
    QJsonDocument doc(json);

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

ConnectionProfile ProfileStorage::loadProfile(const QString& profileName)
{
    if (!m_isValid || profileName.isEmpty()) {
        return ConnectionProfile();
    }

    QString filePath = profileFilePath(profileName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return ConnectionProfile();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        return ConnectionProfile();
    }

    return ConnectionProfile::fromJson(doc.object());
}

QList<ConnectionProfile> ProfileStorage::loadAllProfiles()
{
    QList<ConnectionProfile> profiles;

    if (!m_isValid) {
        return profiles;
    }

    QStringList names = listProfiles();

    for (const QString& name : names) {
        ConnectionProfile profile = loadProfile(name);
        if (profile.isValid()) {
            profiles.append(profile);
        }
    }

    return profiles;
}

bool ProfileStorage::deleteProfile(const QString& profileName)
{
    if (!m_isValid || profileName.isEmpty()) {
        return false;
    }

    QString filePath = profileFilePath(profileName);
    return QFile::remove(filePath);
}

QStringList ProfileStorage::listProfiles()
{
    QStringList profiles;

    if (!m_isValid) {
        return profiles;
    }

    QDir dir(m_storagePath);
    QStringList filters;
    filters << "*.json";

    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QString profileName = fileInfo.baseName();
        profiles.append(profileName);
    }

    return profiles;
}

QString ProfileStorage::defaultStoragePath()
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appData + "/profiles";
}

bool ProfileStorage::ensureStorageDirectory()
{
    QDir dir(m_storagePath);

    if (!dir.exists()) {
        return dir.mkpath(".");
    }

    return true;
}

QString ProfileStorage::profileFilePath(const QString& profileName) const
{
    QString sanitized = sanitizeProfileName(profileName);
    return m_storagePath + "/" + sanitized + ".json";
}

QString ProfileStorage::sanitizeProfileName(const QString& name) const
{
    QString sanitized = name;

    // Remove/replace characters that are problematic in filenames
    sanitized.replace("/", "_");
    sanitized.replace("\\", "_");
    sanitized.replace(":", "_");
    sanitized.replace("*", "_");
    sanitized.replace("?", "_");
    sanitized.replace("\"", "_");
    sanitized.replace("<", "_");
    sanitized.replace(">", "_");
    sanitized.replace("|", "_");

    return sanitized;
}
