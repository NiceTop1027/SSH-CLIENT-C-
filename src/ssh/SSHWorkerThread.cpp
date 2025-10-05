#include "SSHWorkerThread.h"
#include <QMutexLocker>

SSHWorkerThread::SSHWorkerThread(SSHConnection* connection, QObject* parent)
    : QThread(parent), m_connection(connection), m_channel(nullptr), m_stopRequested(false),
      m_running(false)
{
}

SSHWorkerThread::~SSHWorkerThread()
{
    stop();
    wait();
    cleanup();
}

void SSHWorkerThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopRequested = true;
    m_writeCondition.wakeAll();
}

void SSHWorkerThread::writeData(const QString& data)
{
    writeData(data.toUtf8());
}

void SSHWorkerThread::writeData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    m_writeQueue.enqueue(data);
    m_writeCondition.wakeOne();
}

void SSHWorkerThread::run()
{
    m_running = true;
    m_stopRequested = false;

    if (!m_connection || !m_connection->isConnected()) {
        emit error("SSH connection not established");
        m_running = false;
        return;
    }

    if (!initializeChannel()) {
        emit error("Failed to initialize SSH channel");
        m_running = false;
        return;
    }

    // Request PTY and shell
    if (!m_channel->requestPty(24, 80)) {
        emit error("Failed to request PTY");
        cleanup();
        m_running = false;
        return;
    }

    if (!m_channel->requestShell()) {
        emit error("Failed to request shell");
        cleanup();
        m_running = false;
        return;
    }

    // Main I/O loop
    while (!m_stopRequested && m_running) {
        // Process write queue
        processWriteQueue();

        // Read data with timeout
        readLoop();

        // Small delay to prevent CPU spinning
        msleep(10);
    }

    cleanup();
    m_running = false;
}

void SSHWorkerThread::readLoop()
{
    if (!m_channel || !m_channel->isOpen()) {
        return;
    }

    // Non-blocking read with short timeout
    QByteArray data = m_channel->readBytes(4096, 50);

    if (!data.isEmpty()) {
        emit dataReceived(data);
    }

    // Check for EOF or channel closed
    if (m_channel->isEof()) {
        emit disconnected();
        m_stopRequested = true;
    }
}

void SSHWorkerThread::processWriteQueue()
{
    QMutexLocker locker(&m_mutex);

    // Process all pending writes
    while (!m_writeQueue.isEmpty()) {
        QByteArray data = m_writeQueue.dequeue();

        locker.unlock();

        if (m_channel && m_channel->isOpen()) {
            int written = m_channel->write(data);
            if (written < 0) {
                emit error("Failed to write data to SSH channel");
            }
        }

        locker.relock();
    }
}

bool SSHWorkerThread::initializeChannel()
{
    if (!m_connection || !m_connection->session()) {
        return false;
    }

    m_channel = new SSHChannel(m_connection->session());

    if (!m_channel->open()) {
        delete m_channel;
        m_channel = nullptr;
        return false;
    }

    return true;
}

void SSHWorkerThread::cleanup()
{
    if (m_channel) {
        m_channel->close();
        delete m_channel;
        m_channel = nullptr;
    }

    // Clear write queue
    QMutexLocker locker(&m_mutex);
    m_writeQueue.clear();
}
