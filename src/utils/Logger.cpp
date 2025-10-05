#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

// Define logging categories
Q_LOGGING_CATEGORY(sshConnection, "ssh.connection")
Q_LOGGING_CATEGORY(sshAuth, "ssh.auth")
Q_LOGGING_CATEGORY(terminal, "terminal")
Q_LOGGING_CATEGORY(ui, "ui")
Q_LOGGING_CATEGORY(storage, "storage")

Logger* Logger::s_instance = nullptr;

Logger& Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return *s_instance;
}

Logger::Logger() : m_maxFileSize(10 * 1024 * 1024), m_maxFiles(5) // 10MB, 5 files
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::initialize()
{
    // Set default log path
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(defaultPath);
    setupFileLogging(defaultPath + "/ssh-client.log");

    // Install message handler
    qInstallMessageHandler(Logger::messageHandler);
}

void Logger::setupFileLogging(const QString& logPath)
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }

    m_logPath = logPath;
    m_logFile.setFileName(getLogFileName(0));

    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        // Log startup
        QTextStream stream(&m_logFile);
        stream << "\n=== Log started at " << QDateTime::currentDateTime().toString(Qt::ISODate)
               << " ===\n";
        stream.flush();
    }
}

void Logger::setMaxFileSize(qint64 maxSize)
{
    m_maxFileSize = maxSize;
}

void Logger::setMaxFiles(int maxFiles)
{
    m_maxFiles = maxFiles;
}

QString Logger::logFilePath() const
{
    return m_logPath;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context,
                             const QString& msg)
{
    Logger& logger = Logger::instance();
    QMutexLocker locker(&logger.m_mutex);

    // Check file size and rotate if needed
    if (logger.m_logFile.isOpen() && logger.m_logFile.size() > logger.m_maxFileSize) {
        logger.rotateLogFiles();
    }

    // Format message
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString typeStr;

    switch (type) {
    case QtDebugMsg:
        typeStr = "DEBUG";
        break;
    case QtInfoMsg:
        typeStr = "INFO ";
        break;
    case QtWarningMsg:
        typeStr = "WARN ";
        break;
    case QtCriticalMsg:
        typeStr = "ERROR";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    }

    QString category = context.category ? QString(context.category) : "default";
    QString logMessage =
        QString("[%1] [%2] [%3] %4").arg(timestamp, typeStr, category, msg);

    // Write to file
    if (logger.m_logFile.isOpen()) {
        QTextStream stream(&logger.m_logFile);
        stream << logMessage << "\n";
        stream.flush();
    }

    // Also output to console in debug builds
#ifdef QT_DEBUG
    fprintf(stderr, "%s\n", logMessage.toLocal8Bit().constData());
#endif

    if (type == QtFatalMsg) {
        abort();
    }
}

void Logger::rotateLogFiles()
{
    m_logFile.close();

    // Remove oldest log file
    QString oldestFile = getLogFileName(m_maxFiles - 1);
    QFile::remove(oldestFile);

    // Rotate existing files
    for (int i = m_maxFiles - 2; i >= 0; i--) {
        QString oldName = getLogFileName(i);
        QString newName = getLogFileName(i + 1);
        QFile::rename(oldName, newName);
    }

    // Open new log file
    m_logFile.setFileName(getLogFileName(0));
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    QTextStream stream(&m_logFile);
    stream << "\n=== Log rotated at " << QDateTime::currentDateTime().toString(Qt::ISODate)
           << " ===\n";
    stream.flush();
}

QString Logger::getLogFileName(int index) const
{
    if (index == 0) {
        return m_logPath;
    }
    return m_logPath + "." + QString::number(index);
}
