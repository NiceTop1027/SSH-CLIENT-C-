#include "ErrorDialog.h"
#include "Logger.h"
#include <QMessageBox>

void ErrorDialog::showError(QWidget* parent, const QString& title, const QString& message,
                             ErrorType type)
{
    QString formattedMessage = formatErrorMessage(message, type);

    // Log the error
    switch (type) {
    case ErrorType::Connection:
        qCritical(sshConnection) << title << ":" << message;
        break;
    case ErrorType::Authentication:
        qCritical(sshAuth) << title << ":" << message;
        break;
    case ErrorType::Network:
        qCritical(sshConnection) << title << ":" << message;
        break;
    case ErrorType::FileOperation:
        qCritical(storage) << title << ":" << message;
        break;
    case ErrorType::Configuration:
        qCritical(ui) << title << ":" << message;
        break;
    default:
        qCritical() << title << ":" << message;
        break;
    }

    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(title);
    msgBox.setText(formattedMessage);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void ErrorDialog::showConnectionError(QWidget* parent, const QString& hostname,
                                       const QString& details)
{
    QString message = QString("Failed to connect to host '%1'\n\nDetails: %2")
                          .arg(hostname, details);
    showError(parent, "Connection Error", message, ErrorType::Connection);
}

void ErrorDialog::showAuthenticationError(QWidget* parent, const QString& username,
                                           const QString& details)
{
    QString message = QString("Authentication failed for user '%1'\n\nDetails: %2")
                          .arg(username, details);
    showError(parent, "Authentication Error", message, ErrorType::Authentication);
}

void ErrorDialog::showNetworkError(QWidget* parent, const QString& details)
{
    QString message = QString("Network error occurred\n\nDetails: %1").arg(details);
    showError(parent, "Network Error", message, ErrorType::Network);
}

void ErrorDialog::showFileError(QWidget* parent, const QString& filename,
                                 const QString& details)
{
    QString message = QString("File operation failed: '%1'\n\nDetails: %2")
                          .arg(filename, details);
    showError(parent, "File Error", message, ErrorType::FileOperation);
}

QString ErrorDialog::formatErrorMessage(const QString& message, ErrorType type)
{
    QString icon = getErrorIcon(type);
    return QString("%1\n\n%2").arg(icon, message);
}

QString ErrorDialog::getErrorIcon(ErrorType type)
{
    switch (type) {
    case ErrorType::Connection:
        return "🔌 Connection Error";
    case ErrorType::Authentication:
        return "🔐 Authentication Error";
    case ErrorType::Network:
        return "🌐 Network Error";
    case ErrorType::FileOperation:
        return "📁 File Error";
    case ErrorType::Configuration:
        return "⚙️ Configuration Error";
    default:
        return "⚠️ Error";
    }
}
