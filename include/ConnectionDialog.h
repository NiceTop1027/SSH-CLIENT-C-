#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "ConnectionProfile.h"
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget* parent = nullptr);
    ~ConnectionDialog();

    // Profile access
    ConnectionProfile getProfile() const;
    void setProfile(const ConnectionProfile& profile);

    // Field access (for testing)
    QString getHostname() const;
    int getPort() const;
    QString getUsername() const;
    QString getPassword() const;

    void setHostname(const QString& hostname);
    void setPort(int port);
    void setUsername(const QString& username);
    void setAuthMethod(ConnectionProfile::AuthMethod method);

    // Validation
    bool validate();

private slots:
    void onAuthMethodChanged(int index);
    void onBrowseKeyFile();
    void onConnect();
    void onCancel();

private:
    void setupUi();
    void setupConnections();
    void updateAuthUI();

    // UI components
    QLineEdit* m_profileNameEdit;
    QLineEdit* m_hostnameEdit;
    QSpinBox* m_portSpin;
    QLineEdit* m_usernameEdit;
    QComboBox* m_authMethodCombo;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_keyFileEdit;
    QPushButton* m_browseButton;
    QPushButton* m_connectButton;
    QPushButton* m_cancelButton;
};

#endif // CONNECTIONDIALOG_H
