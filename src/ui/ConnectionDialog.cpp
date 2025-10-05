#include "ConnectionDialog.h"
#include "Logger.h"
#include "ErrorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QFile>

ConnectionDialog::ConnectionDialog(QWidget* parent) : QDialog(parent)
{
    setupUi();
    setupConnections();
    setWindowTitle("New SSH Connection");
    resize(450, 350);
}

ConnectionDialog::~ConnectionDialog()
{
}

ConnectionProfile ConnectionDialog::getProfile() const
{
    QString profileName = m_profileNameEdit->text();
    if (profileName.isEmpty()) {
        profileName = m_hostnameEdit->text();
    }

    ConnectionProfile profile(profileName, m_hostnameEdit->text(), m_portSpin->value(),
                              m_usernameEdit->text());

    if (m_authMethodCombo->currentIndex() == 1) { // Public Key
        profile.setAuthMethod(ConnectionProfile::AuthMethod::PublicKey);
        profile.setKeyFilePath(m_keyFileEdit->text());
    } else {
        profile.setAuthMethod(ConnectionProfile::AuthMethod::Password);
    }

    return profile;
}

void ConnectionDialog::setProfile(const ConnectionProfile& profile)
{
    m_profileNameEdit->setText(profile.profileName());
    m_hostnameEdit->setText(profile.hostname());
    m_portSpin->setValue(profile.port());
    m_usernameEdit->setText(profile.username());

    if (profile.authMethod() == ConnectionProfile::AuthMethod::PublicKey) {
        m_authMethodCombo->setCurrentIndex(1);
        m_keyFileEdit->setText(profile.keyFilePath());
    } else {
        m_authMethodCombo->setCurrentIndex(0);
    }

    updateAuthUI();
}

QString ConnectionDialog::getHostname() const
{
    return m_hostnameEdit->text();
}

int ConnectionDialog::getPort() const
{
    return m_portSpin->value();
}

QString ConnectionDialog::getUsername() const
{
    return m_usernameEdit->text();
}

void ConnectionDialog::setHostname(const QString& hostname)
{
    m_hostnameEdit->setText(hostname);
}

void ConnectionDialog::setPort(int port)
{
    m_portSpin->setValue(port);
}

void ConnectionDialog::setUsername(const QString& username)
{
    m_usernameEdit->setText(username);
}

void ConnectionDialog::setAuthMethod(ConnectionProfile::AuthMethod method)
{
    if (method == ConnectionProfile::AuthMethod::PublicKey) {
        m_authMethodCombo->setCurrentIndex(1);
    } else {
        m_authMethodCombo->setCurrentIndex(0);
    }
    updateAuthUI();
}

bool ConnectionDialog::validate()
{
    if (m_hostnameEdit->text().isEmpty()) {
        qWarning(ui) << "Connection validation failed: hostname is empty";
        ErrorDialog::showError(this, "Validation Error", "Hostname cannot be empty",
                               ErrorDialog::ErrorType::Configuration);
        return false;
    }

    if (m_portSpin->value() <= 0 || m_portSpin->value() > 65535) {
        qWarning(ui) << "Connection validation failed: invalid port" << m_portSpin->value();
        ErrorDialog::showError(this, "Validation Error", "Port must be between 1 and 65535",
                               ErrorDialog::ErrorType::Configuration);
        return false;
    }

    if (m_usernameEdit->text().isEmpty()) {
        qWarning(ui) << "Connection validation failed: username is empty";
        ErrorDialog::showError(this, "Validation Error", "Username cannot be empty",
                               ErrorDialog::ErrorType::Configuration);
        return false;
    }

    if (m_authMethodCombo->currentIndex() == 1) { // Public Key
        if (m_keyFileEdit->text().isEmpty()) {
            qWarning(ui) << "Connection validation failed: key file path is empty";
            ErrorDialog::showError(this, "Validation Error", "Key file path cannot be empty",
                                   ErrorDialog::ErrorType::Configuration);
            return false;
        }

        QFile keyFile(m_keyFileEdit->text());
        if (!keyFile.exists()) {
            qWarning(ui) << "Connection validation failed: key file does not exist:"
                         << m_keyFileEdit->text();
            ErrorDialog::showFileError(this, m_keyFileEdit->text(), "Key file does not exist");
            return false;
        }
    }

    qInfo(ui) << "Connection validation passed for host:" << m_hostnameEdit->text();
    return true;
}

void ConnectionDialog::onAuthMethodChanged(int index)
{
    updateAuthUI();
}

void ConnectionDialog::onBrowseKeyFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select SSH Private Key",
                                                     QDir::homePath() + "/.ssh",
                                                     "All Files (*)");

    if (!fileName.isEmpty()) {
        m_keyFileEdit->setText(fileName);
    }
}

void ConnectionDialog::onConnect()
{
    if (validate()) {
        accept();
    }
}

void ConnectionDialog::onCancel()
{
    reject();
}

void ConnectionDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Connection details group
    QGroupBox* connectionGroup = new QGroupBox("Connection Details");
    connectionGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QFormLayout* formLayout = new QFormLayout(connectionGroup);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_profileNameEdit = new QLineEdit();
    m_profileNameEdit->setPlaceholderText("Optional profile name");
    m_profileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("Profile Name:", m_profileNameEdit);

    m_hostnameEdit = new QLineEdit();
    m_hostnameEdit->setObjectName("hostnameEdit");
    m_hostnameEdit->setPlaceholderText("example.com or 192.168.1.100");
    m_hostnameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("Hostname:", m_hostnameEdit);

    m_portSpin = new QSpinBox();
    m_portSpin->setObjectName("portSpin");
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(22);
    m_portSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("Port:", m_portSpin);

    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setObjectName("usernameEdit");
    m_usernameEdit->setPlaceholderText("username");
    m_usernameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("Username:", m_usernameEdit);

    mainLayout->addWidget(connectionGroup);

    // Authentication group
    QGroupBox* authGroup = new QGroupBox("Authentication");
    authGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QFormLayout* authLayout = new QFormLayout(authGroup);
    authLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_authMethodCombo = new QComboBox();
    m_authMethodCombo->setObjectName("authMethodCombo");
    m_authMethodCombo->addItem("Password");
    m_authMethodCombo->addItem("Public Key");
    m_authMethodCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    authLayout->addRow("Method:", m_authMethodCombo);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setObjectName("passwordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter password");
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    authLayout->addRow("Password:", m_passwordEdit);

    QHBoxLayout* keyLayout = new QHBoxLayout();
    m_keyFileEdit = new QLineEdit();
    m_keyFileEdit->setObjectName("keyFileEdit");
    m_keyFileEdit->setPlaceholderText("/path/to/private/key");
    m_keyFileEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    keyLayout->addWidget(m_keyFileEdit);
    keyLayout->addWidget(m_browseButton);
    authLayout->addRow("Key File:", keyLayout);

    mainLayout->addWidget(authGroup);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_connectButton = new QPushButton("Connect");
    m_connectButton->setObjectName("connectButton");
    m_connectButton->setDefault(true);
    m_connectButton->setMinimumWidth(100);
    buttonLayout->addWidget(m_connectButton);

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setMinimumWidth(100);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Set minimum dialog size
    setMinimumSize(400, 300);

    // Initial UI state
    updateAuthUI();
}

void ConnectionDialog::setupConnections()
{
    connect(m_authMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ConnectionDialog::onAuthMethodChanged);
    connect(m_browseButton, &QPushButton::clicked, this, &ConnectionDialog::onBrowseKeyFile);
    connect(m_connectButton, &QPushButton::clicked, this, &ConnectionDialog::onConnect);
    connect(m_cancelButton, &QPushButton::clicked, this, &ConnectionDialog::onCancel);
}

void ConnectionDialog::updateAuthUI()
{
    bool isKeyAuth = (m_authMethodCombo->currentIndex() == 1);

    m_passwordEdit->setVisible(!isKeyAuth);
    m_keyFileEdit->setVisible(isKeyAuth);
    m_browseButton->setVisible(isKeyAuth);

    // Update form layout visibility
    QWidget* authGroupWidget = m_passwordEdit->parentWidget();
    if (authGroupWidget) {
        QFormLayout* authLayout = qobject_cast<QFormLayout*>(authGroupWidget->layout());
        if (authLayout) {
            QLabel* passwordLabel = qobject_cast<QLabel*>(authLayout->labelForField(m_passwordEdit));
            if (passwordLabel) {
                passwordLabel->setVisible(!isKeyAuth);
            }

            QLabel* keyLabel = qobject_cast<QLabel*>(authLayout->labelForField(m_keyFileEdit));
            if (keyLabel) {
                keyLabel->setVisible(isKeyAuth);
            }
        }
    }
}
