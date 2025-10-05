#ifndef LOGGER_H
#define LOGGER_H

#include <QLoggingCategory>
#include <QString>
#include <QFile>
#include <QMutex>

// Logging categories
Q_DECLARE_LOGGING_CATEGORY(sshConnection)
Q_DECLARE_LOGGING_CATEGORY(sshAuth)
Q_DECLARE_LOGGING_CATEGORY(terminal)
Q_DECLARE_LOGGING_CATEGORY(ui)
Q_DECLARE_LOGGING_CATEGORY(storage)

class Logger {
public:
    static Logger& instance();

    void initialize();
    void setupFileLogging(const QString& logPath = QString());
    void setMaxFileSize(qint64 maxSize);
    void setMaxFiles(int maxFiles);

    QString logFilePath() const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void messageHandler(QtMsgType type, const QMessageLogContext& context,
                                const QString& msg);
    void rotateLogFiles();
    QString getLogFileName(int index = 0) const;

    QFile m_logFile;
    QString m_logPath;
    qint64 m_maxFileSize;
    int m_maxFiles;
    QMutex m_mutex;
    static Logger* s_instance;
};

#endif // LOGGER_H
