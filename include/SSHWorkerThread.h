#ifndef SSHWORKERTHREAD_H
#define SSHWORKERTHREAD_H

#include "SSHConnection.h"
#include "SSHChannel.h"
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QByteArray>

class SSHWorkerThread : public QThread {
    Q_OBJECT

public:
    explicit SSHWorkerThread(SSHConnection* connection, QObject* parent = nullptr);
    ~SSHWorkerThread();

    // Thread control
    void stop();

    // Data transmission
    void writeData(const QString& data);
    void writeData(const QByteArray& data);

signals:
    void dataReceived(const QByteArray& data);
    void error(const QString& message);
    void disconnected();

protected:
    void run() override;

private:
    void readLoop();
    void processWriteQueue();
    bool initializeChannel();
    void cleanup();

    SSHConnection* m_connection;
    SSHChannel* m_channel;
    QMutex m_mutex;
    QQueue<QByteArray> m_writeQueue;
    QWaitCondition m_writeCondition;
    bool m_stopRequested;
    bool m_running;
};

#endif // SSHWORKERTHREAD_H
