#include "ConnectionProfile.h"

ConnectionProfile::ConnectionProfile()
    : m_port(22), m_authMethod(AuthMethod::Password)
{
}

ConnectionProfile::ConnectionProfile(const QString& profileName, const QString& hostname, int port,
                                     const QString& username)
    : m_profileName(profileName), m_hostname(hostname), m_port(port), m_username(username),
      m_authMethod(AuthMethod::Password)
{
}

QString ConnectionProfile::profileName() const
{
    return m_profileName;
}

QString ConnectionProfile::hostname() const
{
    return m_hostname;
}

int ConnectionProfile::port() const
{
    return m_port;
}

QString ConnectionProfile::username() const
{
    return m_username;
}

ConnectionProfile::AuthMethod ConnectionProfile::authMethod() const
{
    return m_authMethod;
}

QString ConnectionProfile::keyFilePath() const
{
    return m_keyFilePath;
}

void ConnectionProfile::setProfileName(const QString& name)
{
    m_profileName = name;
}

void ConnectionProfile::setHostname(const QString& host)
{
    m_hostname = host;
}

void ConnectionProfile::setPort(int port)
{
    m_port = port;
}

void ConnectionProfile::setUsername(const QString& user)
{
    m_username = user;
}

void ConnectionProfile::setAuthMethod(AuthMethod method)
{
    m_authMethod = method;
}

void ConnectionProfile::setKeyFilePath(const QString& path)
{
    m_keyFilePath = path;
}

bool ConnectionProfile::isValid() const
{
    // Hostname must not be empty
    if (m_hostname.isEmpty()) {
        return false;
    }

    // Port must be in valid range
    if (m_port <= 0 || m_port > 65535) {
        return false;
    }

    // Username must not be empty
    if (m_username.isEmpty()) {
        return false;
    }

    return true;
}

QJsonObject ConnectionProfile::toJson() const
{
    QJsonObject json;
    json["profileName"] = m_profileName;
    json["hostname"] = m_hostname;
    json["port"] = m_port;
    json["username"] = m_username;
    json["authMethod"] = (m_authMethod == AuthMethod::Password) ? "password" : "publickey";
    json["keyFilePath"] = m_keyFilePath;
    return json;
}

ConnectionProfile ConnectionProfile::fromJson(const QJsonObject& json)
{
    ConnectionProfile profile;
    profile.setProfileName(json["profileName"].toString());
    profile.setHostname(json["hostname"].toString());
    profile.setPort(json["port"].toInt(22));
    profile.setUsername(json["username"].toString());

    QString authMethodStr = json["authMethod"].toString("password");
    if (authMethodStr == "publickey") {
        profile.setAuthMethod(AuthMethod::PublicKey);
    } else {
        profile.setAuthMethod(AuthMethod::Password);
    }

    profile.setKeyFilePath(json["keyFilePath"].toString());

    return profile;
}
