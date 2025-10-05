#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QString>
#include <QWidget>

class ErrorDialog {
public:
    enum class ErrorType {
        Connection,
        Authentication,
        Network,
        FileOperation,
        Configuration,
        Unknown
    };

    static void showError(QWidget* parent, const QString& title, const QString& message,
                          ErrorType type = ErrorType::Unknown);

    static void showConnectionError(QWidget* parent, const QString& hostname, const QString& details);
    static void showAuthenticationError(QWidget* parent, const QString& username, const QString& details);
    static void showNetworkError(QWidget* parent, const QString& details);
    static void showFileError(QWidget* parent, const QString& filename, const QString& details);

    static QString formatErrorMessage(const QString& message, ErrorType type);

private:
    ErrorDialog() = delete;
    static QString getErrorIcon(ErrorType type);
};

#endif // ERRORDIALOG_H
