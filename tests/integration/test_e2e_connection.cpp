#include "SSHConnection.h"
#include "SSHAuthenticator.h"
#include "SSHChannel.h"
#include "ConnectionProfile.h"
#include "Logger.h"
#include <QTest>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include <QProcessEnvironment>

class TestE2EConnection : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // End-to-end test cases
    void testPasswordAuthenticationFlow();
    void testPublicKeyAuthenticationFlow();
    void testConnectionFailureScenarios();
    void testChannelCommunication();
    void testGracefulDisconnect();
    void testConcurrentConnections();
    void testReconnectionAfterFailure();
    void testLongRunningSession();

private:
    void setupTestEnvironment();
    bool hasTestServer() const;
    QString getTestHost() const;
    int getTestPort() const;
    QString getTestUsername() const;
    QString getTestPassword() const;
    QString getTestKeyPath() const;

    bool m_hasTestServer;
};

void TestE2EConnection::initTestCase()
{
    // Initialize logging for tests
    Logger::instance().initialize();
    qInfo() << "Starting E2E integration tests";

    setupTestEnvironment();
}

void TestE2EConnection::cleanupTestCase()
{
    qInfo() << "E2E integration tests complete";
}

void TestE2EConnection::init()
{
    // Setup before each test
}

void TestE2EConnection::cleanup()
{
    // Cleanup after each test
}

void TestE2EConnection::setupTestEnvironment()
{
    // Check if test SSH server is available
    // This would typically check environment variables or a test configuration file
    m_hasTestServer = false;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("SSH_TEST_HOST") && env.contains("SSH_TEST_USER")) {
        m_hasTestServer = true;
        qInfo() << "Test SSH server detected:" << getTestHost();
    } else {
        qWarning() << "No test SSH server configured - skipping live tests";
        qWarning() << "Set SSH_TEST_HOST, SSH_TEST_USER, SSH_TEST_PASSWORD to enable";
    }
}

bool TestE2EConnection::hasTestServer() const
{
    return m_hasTestServer;
}

QString TestE2EConnection::getTestHost() const
{
    return QProcessEnvironment::systemEnvironment().value("SSH_TEST_HOST", "localhost");
}

int TestE2EConnection::getTestPort() const
{
    return QProcessEnvironment::systemEnvironment().value("SSH_TEST_PORT", "22").toInt();
}

QString TestE2EConnection::getTestUsername() const
{
    return QProcessEnvironment::systemEnvironment().value("SSH_TEST_USER");
}

QString TestE2EConnection::getTestPassword() const
{
    return QProcessEnvironment::systemEnvironment().value("SSH_TEST_PASSWORD");
}

QString TestE2EConnection::getTestKeyPath() const
{
    return QProcessEnvironment::systemEnvironment().value("SSH_TEST_KEY_PATH");
}

void TestE2EConnection::testPasswordAuthenticationFlow()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured or password not provided");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    profile.setAuthMethod(ConnectionProfile::AuthMethod::Password);

    SSHConnection connection(profile);
    SSHAuthenticator authenticator(profile, getTestPassword());
    connection.setAuthenticator(&authenticator);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    QSignalSpy errorSpy(&connection, &SSHConnection::error);

    connection.connectToHost();

    // Wait for connection (with timeout)
    QVERIFY(connectedSpy.wait(10000));
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(connection.isConnected());

    connection.disconnect();
}

void TestE2EConnection::testPublicKeyAuthenticationFlow()
{
    if (!hasTestServer() || getTestKeyPath().isEmpty()) {
        QSKIP("Test SSH server not configured or key path not provided");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    profile.setAuthMethod(ConnectionProfile::AuthMethod::PublicKey);
    profile.setKeyFilePath(getTestKeyPath());

    SSHConnection connection(profile);
    SSHAuthenticator authenticator(profile, QString(), getTestKeyPath());
    connection.setAuthenticator(&authenticator);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    QSignalSpy errorSpy(&connection, &SSHConnection::error);

    connection.connectToHost();

    QVERIFY(connectedSpy.wait(10000));
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(connection.isConnected());

    connection.disconnect();
}

void TestE2EConnection::testConnectionFailureScenarios()
{
    // Test 1: Invalid hostname
    {
        ConnectionProfile profile("test", "invalid.host.invalid", 22, "user");
        SSHConnection connection(profile);

        QSignalSpy errorSpy(&connection, &SSHConnection::error);
        connection.connectToHost();

        QVERIFY(errorSpy.wait(5000));
        QVERIFY(errorSpy.count() > 0);
        QCOMPARE(connection.lastErrorType(), SSHConnection::ErrorType::InvalidHost);
    }

    // Test 2: Connection refused (wrong port)
    {
        ConnectionProfile profile("test", getTestHost(), 9999, getTestUsername());
        SSHConnection connection(profile);

        QSignalSpy errorSpy(&connection, &SSHConnection::error);
        connection.connectToHost();

        QVERIFY(errorSpy.wait(5000));
        QVERIFY(errorSpy.count() > 0);
        QVERIFY(connection.lastErrorType() == SSHConnection::ErrorType::ConnectionRefused ||
                connection.lastErrorType() == SSHConnection::ErrorType::NetworkError);
    }

    // Test 3: Authentication failure (wrong password)
    if (hasTestServer()) {
        ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
        profile.setAuthMethod(ConnectionProfile::AuthMethod::Password);

        SSHConnection connection(profile);
        SSHAuthenticator authenticator(profile, "wrong_password");
        connection.setAuthenticator(&authenticator);

        QSignalSpy authFailedSpy(&connection, &SSHConnection::authenticationFailed);
        connection.connectToHost();

        QVERIFY(authFailedSpy.wait(10000));
        QVERIFY(authFailedSpy.count() > 0);
    }
}

void TestE2EConnection::testChannelCommunication()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    SSHConnection connection(profile);
    SSHAuthenticator authenticator(profile, getTestPassword());
    connection.setAuthenticator(&authenticator);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    connection.connectToHost();
    QVERIFY(connectedSpy.wait(10000));

    // Create channel and execute command
    SSHChannel channel(connection.session());
    QVERIFY(channel.open());
    QVERIFY(channel.requestPty());
    QVERIFY(channel.requestShell());

    // Send command
    QString command = "echo 'test'\n";
    QVERIFY(channel.write(command.toUtf8()));

    // Read response
    QByteArray response = channel.readBytes(1024, 2000);
    QVERIFY(!response.isEmpty());

    channel.close();
    connection.disconnect();
}

void TestE2EConnection::testGracefulDisconnect()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    SSHConnection connection(profile);
    SSHAuthenticator authenticator(profile, getTestPassword());
    connection.setAuthenticator(&authenticator);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    QSignalSpy disconnectedSpy(&connection, &SSHConnection::disconnected);

    connection.connectToHost();
    QVERIFY(connectedSpy.wait(10000));

    connection.disconnect();
    QVERIFY(disconnectedSpy.wait(5000));
    QCOMPARE(disconnectedSpy.count(), 1);
    QVERIFY(!connection.isConnected());
}

void TestE2EConnection::testConcurrentConnections()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured");
    }

    const int numConnections = 3;
    QVector<SSHConnection*> connections;
    QVector<SSHAuthenticator*> authenticators;

    // Create multiple connections
    for (int i = 0; i < numConnections; ++i) {
        ConnectionProfile profile(QString("test%1").arg(i), getTestHost(), getTestPort(),
                                  getTestUsername());
        SSHConnection* conn = new SSHConnection(profile);
        SSHAuthenticator* auth = new SSHAuthenticator(profile, getTestPassword());
        conn->setAuthenticator(auth);

        connections.append(conn);
        authenticators.append(auth);
    }

    // Connect all
    QVector<QSignalSpy*> spies;
    for (auto* conn : connections) {
        QSignalSpy* spy = new QSignalSpy(conn, &SSHConnection::connected);
        spies.append(spy);
        conn->connectToHost();
    }

    // Wait for all connections
    for (auto* spy : spies) {
        QVERIFY(spy->wait(10000));
    }

    // Verify all connected
    for (auto* conn : connections) {
        QVERIFY(conn->isConnected());
    }

    // Cleanup
    for (auto* conn : connections) {
        conn->disconnect();
        delete conn;
    }
    for (auto* auth : authenticators) {
        delete auth;
    }
    for (auto* spy : spies) {
        delete spy;
    }
}

void TestE2EConnection::testReconnectionAfterFailure()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    SSHConnection connection(profile);

    // First attempt with wrong password
    SSHAuthenticator wrongAuth(profile, "wrong_password");
    connection.setAuthenticator(&wrongAuth);

    QSignalSpy errorSpy(&connection, &SSHConnection::authenticationFailed);
    connection.connectToHost();
    QVERIFY(errorSpy.wait(10000));

    // Second attempt with correct password
    SSHAuthenticator correctAuth(profile, getTestPassword());
    connection.setAuthenticator(&correctAuth);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    connection.connectToHost();
    QVERIFY(connectedSpy.wait(10000));
    QVERIFY(connection.isConnected());

    connection.disconnect();
}

void TestE2EConnection::testLongRunningSession()
{
    if (!hasTestServer() || getTestPassword().isEmpty()) {
        QSKIP("Test SSH server not configured");
    }

    ConnectionProfile profile("test", getTestHost(), getTestPort(), getTestUsername());
    SSHConnection connection(profile);
    SSHAuthenticator authenticator(profile, getTestPassword());
    connection.setAuthenticator(&authenticator);

    QSignalSpy connectedSpy(&connection, &SSHConnection::connected);
    connection.connectToHost();
    QVERIFY(connectedSpy.wait(10000));

    // Keep connection alive for 30 seconds
    QTest::qWait(30000);

    // Verify still connected
    QVERIFY(connection.isConnected());

    connection.disconnect();
}

QTEST_MAIN(TestE2EConnection)
#include "test_e2e_connection.moc"
