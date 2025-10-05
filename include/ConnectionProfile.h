#ifndef CONNECTIONPROFILE_H
#define CONNECTIONPROFILE_H

#include <QJsonObject>
#include <QString>

class ConnectionProfile {
public:
    enum class AuthMethod { Password, PublicKey };

    // Constructors
    ConnectionProfile();
    ConnectionProfile(const QString& profileName, const QString& hostname, int port,
                      const QString& username);

    // Getters
    QString profileName() const;
    QString hostname() const;
    int port() const;
    QString username() const;
    AuthMethod authMethod() const;
    QString keyFilePath() const;

    // Setters
    void setProfileName(const QString& name);
    void setHostname(const QString& host);
    void setPort(int port);
    void setUsername(const QString& user);
    void setAuthMethod(AuthMethod method);
    void setKeyFilePath(const QString& path);

    // Validation
    bool isValid() const;

    // Serialization
    QJsonObject toJson() const;
    static ConnectionProfile fromJson(const QJsonObject& json);

private:
    QString m_profileName;
    QString m_hostname;
    int m_port;
    QString m_username;
    AuthMethod m_authMethod;
    QString m_keyFilePath;
};

#endif // CONNECTIONPROFILE_H
