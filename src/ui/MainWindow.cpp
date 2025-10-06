#include "MainWindow.h"
#include "ConnectionDialog.h"
#include "PreferencesDialog.h"
#include "SimplePasswordDialog.h"
#include "AppSettings.h"
#include "SSHConnection.h"
#include "SSHAuthenticator.h"
#include "SSHWorkerThread.h"
#include "Logger.h"
#include "ErrorDialog.h"
#include "TerminalView.h"
#include "CredentialManager.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTabBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_tabWidget(nullptr), m_welcomeWidget(nullptr), m_savedConnectionsList(nullptr)
{
    qInfo(ui) << "MainWindow initializing";

    // Profile storage is now handled via JSON files directly
    // m_profileStorage = new ProfileStorage();

    setupUi();
    createActions();
    createMenus();
    createStatusBar();
    setupConnections();

    // Set window properties
    setWindowTitle("SSH Client");

    // Load window size from settings
    AppSettings* settings = AppSettings::instance();
    if (settings->rememberWindowSize()) {
        resize(settings->lastWindowSize());
    } else {
        resize(1024, 768);
    }
    setMinimumSize(800, 600);

    // Apply clean minimal theme
    setStyleSheet(R"(
        QMainWindow {
            background: #1E1E1E;
        }
        QMenuBar {
            background: #252526;
            color: #CCCCCC;
            border-bottom: 1px solid #3C3C3C;
            spacing: 3px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 8px 12px;
        }
        QMenuBar::item:selected {
            background: #37373D;
        }
        QMenu {
            background: #252526;
            color: #CCCCCC;
            border: 1px solid #3C3C3C;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 30px;
            border-radius: 3px;
        }
        QMenu::item:selected {
            background: #094771;
        }
        QTabWidget::pane {
            border: none;
            background: #1E1E1E;
        }
        QTabBar::tab {
            background: #2D2D30;
            color: #969696;
            padding: 10px 16px;
            margin-right: 1px;
            border: none;
        }
        QTabBar::tab:selected {
            background: #1E1E1E;
            color: #FFFFFF;
            border-top: 2px solid #007ACC;
        }
        QTabBar::tab:hover {
            background: #3E3E42;
            color: #CCCCCC;
        }
        QStatusBar {
            background: #007ACC;
            color: #FFFFFF;
            border: none;
        }
    )");

    // Apply window opacity
    setWindowOpacity(settings->terminalOpacity());

    // Connect to settings changes
    connect(settings, &AppSettings::settingsChanged, this, &MainWindow::onSettingsChanged);

    // Ensure window is visible and raised
    raise();
    activateWindow();

    qInfo(ui) << "MainWindow initialized successfully";
}

MainWindow::~MainWindow()
{
    qInfo(ui) << "MainWindow shutting down, cleaning up" << m_tabs.size() << "connections";

    // Save window size
    AppSettings* settings = AppSettings::instance();
    if (settings->rememberWindowSize()) {
        settings->setLastWindowSize(size());
        settings->save();
    }

    // Cleanup connections
    for (int i = 0; i < m_tabs.size(); ++i) {
        auto& tabData = m_tabs[i];
        qDebug(ui) << "Cleaning up tab" << i;

        if (tabData.worker && tabData.worker->isRunning()) {
            qDebug(ui) << "Stopping worker thread for tab" << i;
            tabData.worker->stop();
            tabData.worker->wait(5000); // Wait up to 5 seconds
            if (tabData.worker->isRunning()) {
                qWarning(ui) << "Worker thread did not stop gracefully for tab" << i;
                tabData.worker->terminate();
                tabData.worker->wait();
            }
        }
        delete tabData.worker;

        if (tabData.connection) {
            qDebug(ui) << "Disconnecting SSH connection for tab" << i;
            tabData.connection->disconnect();
        }
        delete tabData.connection;
    }

    // delete m_profileStorage;
    qInfo(ui) << "MainWindow cleanup complete";
}

void MainWindow::addNewTab(const QString& title)
{
    TabData tabData;

    // Create terminal directly (no split container for now)
    tabData.terminal = new TerminalView();
    tabData.splitContainer = nullptr;

    // Apply current settings to new terminal
    AppSettings* settings = AppSettings::instance();
    tabData.terminal->setBackgroundImage(settings->terminalBackgroundImage());
    tabData.terminal->setBackgroundImageOpacity(settings->backgroundImageOpacity());
    QFont font(settings->terminalFontFamily(), settings->terminalFontSize());
    tabData.terminal->setCustomFont(font);
    tabData.terminal->setCustomColors(settings->terminalForegroundColor(),
                                      settings->terminalBackgroundColor());

    tabData.connection = nullptr;
    tabData.worker = nullptr;

    m_tabs.append(tabData);

    int index = m_tabWidget->addTab(tabData.terminal, title);
    m_tabWidget->setCurrentIndex(index);
}

void MainWindow::closeCurrentTab()
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0) {
        onTabCloseRequested(index);
    }
}

TerminalView* MainWindow::currentTerminal()
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        return m_tabs[index].terminal;
    }
    return nullptr;
}

QTabWidget* MainWindow::tabWidget() const
{
    return m_tabWidget;
}

void MainWindow::showStatusMessage(const QString& message, int timeout)
{
    statusBar()->showMessage(message, timeout);
}

void MainWindow::onNewConnection()
{
    ConnectionDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        ConnectionProfile profile = dialog.getProfile();

        // Get password from dialog if password authentication
        QString password;
        if (profile.authMethod() == ConnectionProfile::AuthMethod::Password) {
            // Find password edit in dialog
            QLineEdit* passwordEdit = dialog.findChild<QLineEdit*>("passwordEdit");
            if (passwordEdit) {
                password = passwordEdit->text();

                // Store password in Keychain
                if (!password.isEmpty()) {
                    CredentialManager* credManager = new CredentialManager();
                    QString service = QString("SSH:%1").arg(profile.hostname());
                    try {
                        credManager->storePassword(service, profile.username(), password);
                        qInfo(ui) << "Password stored in Keychain for" << profile.username() << "@" << profile.hostname();
                    } catch (...) {
                        qWarning(ui) << "Failed to store password in Keychain";
                    }
                    delete credManager;
                }
            }
        }

        // Save profile for future use (before hiding welcome tab)
        saveCurrentProfile(profile);

        // Start connection (this will hide welcome tab)
        handleConnectionRequest(profile, password);
    }
}

void MainWindow::onCloseTab()
{
    closeCurrentTab();
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onCopy()
{
    TerminalView* terminal = currentTerminal();
    if (terminal) {
        // TODO: Implement text selection in TerminalView
        // QClipboard* clipboard = QApplication::clipboard();
        // clipboard->setText(terminal->getSelectedText());
    }
}

void MainWindow::onPaste()
{
    TerminalView* terminal = currentTerminal();
    if (terminal) {
        QClipboard* clipboard = QApplication::clipboard();
        QString text = clipboard->text();
        emit terminal->sendData(text);
    }
}

void MainWindow::onPreferences()
{
    PreferencesDialog dialog(this);
    dialog.exec();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About SSH Client",
                       "SSH Client\n\n"
                       "A cross-platform SSH terminal client built with Qt and libssh.\n\n"
                       "Features:\n"
                       "- Multiple concurrent connections\n"
                       "- Password and key authentication\n"
                       "- Terminal emulation with ANSI support\n"
                       "- Customizable appearance with background images\n"
                       "- Font and color customization\n"
                       "- Window transparency support");
}

void MainWindow::onSettingsChanged()
{
    AppSettings* settings = AppSettings::instance();
    setWindowOpacity(settings->terminalOpacity());
}

void MainWindow::onTabCloseRequested(int index)
{
    if (index < 0 || index >= m_tabs.size()) {
        return;
    }

    qInfo(ui) << "Closing tab" << index;
    TabData& tabData = m_tabs[index];

    // Stop worker thread
    if (tabData.worker && tabData.worker->isRunning()) {
        qDebug(ui) << "Stopping worker thread for tab" << index;
        tabData.worker->stop();
        tabData.worker->wait(3000); // Wait up to 3 seconds
        if (tabData.worker->isRunning()) {
            qWarning(ui) << "Forcefully terminating worker thread for tab" << index;
            tabData.worker->terminate();
            tabData.worker->wait();
        }
        delete tabData.worker;
        tabData.worker = nullptr;
    }

    // Delete connection
    if (tabData.connection) {
        qDebug(ui) << "Disconnecting SSH connection for tab" << index;
        tabData.connection->disconnect();
        delete tabData.connection;
        tabData.connection = nullptr;
    }

    // Remove tab
    m_tabWidget->removeTab(index);
    m_tabs.removeAt(index);
    qInfo(ui) << "Tab" << index << "closed successfully";

    // Show welcome tab if no more connection tabs
    if (m_tabs.isEmpty()) {
        showWelcomeTab();
    }
}

void MainWindow::handleConnectionRequest(const ConnectionProfile& profile, const QString& password)
{
    qDebug(ui) << "=== handleConnectionRequest START ===";
    qDebug(ui) << "Profile:" << profile.profileName() << profile.username() << profile.hostname() << profile.port();

    // Hide welcome tab
    hideWelcomeTab();
    qDebug(ui) << "Welcome tab hidden";

    // Create new terminal tab
    addNewTab(profile.profileName());
    qDebug(ui) << "New tab added, total tabs:" << m_tabs.size();

    int index = m_tabWidget->currentIndex();
    qDebug(ui) << "Current tab index:" << index << "m_tabs.size:" << m_tabs.size();

    if (index < 0 || index >= m_tabs.size()) {
        qCritical(ui) << "CRITICAL: Invalid index" << index << "for m_tabs.size" << m_tabs.size();
        return;
    }

    TabData& tabData = m_tabs[index];
    qDebug(ui) << "Got TabData, terminal:" << (void*)tabData.terminal;

    if (!tabData.terminal) {
        qCritical(ui) << "CRITICAL: Terminal is null!";
        return;
    }

    // Create connection
    qDebug(ui) << "Creating SSHConnection...";
    tabData.connection = new SSHConnection(profile);
    qDebug(ui) << "SSHConnection created:" << (void*)tabData.connection;

    // Create authenticator with credentials
    qDebug(ui) << "Creating authenticator...";
    // Only pass keyFilePath if using PublicKey auth, otherwise pass empty string
    QString keyPath;
    if (profile.authMethod() == ConnectionProfile::AuthMethod::PublicKey) {
        keyPath = profile.keyFilePath();
    }
    SSHAuthenticator* authenticator = new SSHAuthenticator(profile, password, keyPath);
    tabData.connection->setAuthenticator(authenticator);
    qDebug(ui) << "Authenticator set";

    // Setup connection signals - use connection pointer to find tab later
    SSHConnection* conn = tabData.connection;
    qDebug(ui) << "Connecting signals...";
    connect(conn, &SSHConnection::connected, this, [this, conn]() {
        qDebug(ui) << "Connected signal received";
        // Find tab by connection pointer
        for (int i = 0; i < m_tabs.size(); i++) {
            if (m_tabs[i].connection == conn) {
                qDebug(ui) << "Found matching tab at index" << i;
                handleConnected(i);
                break;
            }
        }
    });
    connect(tabData.connection, &SSHConnection::disconnected, this, &MainWindow::handleDisconnected);
    connect(tabData.connection, &SSHConnection::error, this, &MainWindow::handleError);
    qDebug(ui) << "Signals connected";

    // Connect to host
    showStatusMessage("Connecting to " + profile.hostname() + "...");
    qDebug(ui) << "Calling connectToHost...";
    tabData.connection->connectToHost();
    qDebug(ui) << "=== handleConnectionRequest END ===";
}

void MainWindow::handleConnected(int tabIndex)
{
    showStatusMessage("Connected", 3000);

    if (tabIndex < 0 || tabIndex >= m_tabs.size()) {
        return;
    }

    TabData& tabData = m_tabs[tabIndex];

    // Create worker thread
    tabData.worker = new SSHWorkerThread(tabData.connection);

    // Connect worker signals - capture tab index
    connect(tabData.worker, &SSHWorkerThread::dataReceived, this,
            [this, tabIndex](const QByteArray& data) {
                handleDataReceived(tabIndex, data);
            });
    connect(tabData.worker, &SSHWorkerThread::error, this, &MainWindow::handleError);
    connect(tabData.worker, &SSHWorkerThread::disconnected, this,
            &MainWindow::handleDisconnected);

    // Connect terminal to worker
    if (tabData.terminal) {
        connect(tabData.terminal, &TerminalView::sendData,
                [worker = tabData.worker](const QString& data) {
                    worker->writeData(data.toUtf8());
                });
    }

    // Start worker
    tabData.worker->start();
}

void MainWindow::handleDisconnected()
{
    showStatusMessage("Disconnected");
}

void MainWindow::handleError(const QString& error)
{
    qWarning(ui) << "Connection error:" << error;
    showStatusMessage("Error: " + error);

    // Find which connection triggered the error
    SSHConnection* connection = qobject_cast<SSHConnection*>(sender());
    if (connection) {
        SSHConnection::ErrorType errorType = connection->lastErrorType();

        // Show appropriate error dialog based on error type
        switch (errorType) {
        case SSHConnection::ErrorType::InvalidHost:
            ErrorDialog::showConnectionError(this, connection->profile().hostname(),
                                             "Invalid hostname or DNS resolution failed");
            break;
        case SSHConnection::ErrorType::ConnectionRefused:
            ErrorDialog::showConnectionError(this, connection->profile().hostname(),
                                             "Connection refused - check if SSH server is running");
            break;
        case SSHConnection::ErrorType::AuthenticationFailed:
            ErrorDialog::showAuthenticationError(this, connection->profile().username(),
                                                  error);
            break;
        case SSHConnection::ErrorType::Timeout:
            ErrorDialog::showNetworkError(this, "Connection timeout - host may be unreachable");
            break;
        case SSHConnection::ErrorType::NetworkError:
            ErrorDialog::showNetworkError(this, error);
            break;
        case SSHConnection::ErrorType::HostKeyVerificationFailed:
            ErrorDialog::showError(this, "Security Error",
                                   "Host key verification failed\n\n" + error,
                                   ErrorDialog::ErrorType::Connection);
            break;
        default:
            ErrorDialog::showError(this, "Connection Error", error,
                                   ErrorDialog::ErrorType::Connection);
            break;
        }
    } else {
        ErrorDialog::showError(this, "Error", error, ErrorDialog::ErrorType::Unknown);
    }
}

void MainWindow::handleDataReceived(int tabIndex, const QByteArray& data)
{
    if (tabIndex >= 0 && tabIndex < m_tabs.size()) {
        // Display on terminal
        TerminalView* terminal = m_tabs[tabIndex].terminal;
        if (terminal) {
            terminal->displayOutput(QString::fromUtf8(data));
        }
    }
}

void MainWindow::setupUi()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    // Set size policy to expand with window
    m_tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Enable document mode for cleaner tabs
    m_tabWidget->setDocumentMode(true);

    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this,
            &MainWindow::onTabCloseRequested);

    // Tab change handler (animations disabled for stability)
    connect(m_tabWidget, &QTabWidget::currentChanged, [](int index) {
        // Just switch tabs without animation for now
        Q_UNUSED(index);
    });

    // Show welcome tab initially (will be hidden when first connection is made)
    showWelcomeTab();
}

void MainWindow::createMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->setObjectName("fileMenu");
    fileMenu->addAction(m_newConnectionAction);
    fileMenu->addAction(m_closeTabAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_preferencesAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->setObjectName("editMenu");
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->setObjectName("helpMenu");
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createActions()
{
    m_newConnectionAction = new QAction("&New Connection...", this);
    m_newConnectionAction->setObjectName("newConnectionAction");
    m_newConnectionAction->setShortcut(QKeySequence::New);
    connect(m_newConnectionAction, &QAction::triggered, this, &MainWindow::onNewConnection);

    m_closeTabAction = new QAction("&Close Tab", this);
    m_closeTabAction->setObjectName("closeTabAction");
    m_closeTabAction->setShortcut(QKeySequence::Close);
    connect(m_closeTabAction, &QAction::triggered, this, &MainWindow::onCloseTab);

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setObjectName("exitAction");
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

    m_copyAction = new QAction("&Copy", this);
    m_copyAction->setObjectName("copyAction");
    m_copyAction->setShortcut(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::onCopy);

    m_pasteAction = new QAction("&Paste", this);
    m_pasteAction->setObjectName("pasteAction");
    m_pasteAction->setShortcut(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, this, &MainWindow::onPaste);

    m_preferencesAction = new QAction("&Preferences...", this);
    m_preferencesAction->setObjectName("preferencesAction");
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::onPreferences);

    m_aboutAction = new QAction("&About", this);
    m_aboutAction->setObjectName("aboutAction");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::setupConnections()
{
    // Additional connections if needed
}

void MainWindow::showWelcomeTab()
{
    if (m_welcomeWidget) {
        return; // Already showing
    }

    m_welcomeWidget = new QWidget(this);
    m_welcomeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_welcomeWidget->setStyleSheet("background: #1E1E1E;");

    QVBoxLayout* layout = new QVBoxLayout(m_welcomeWidget);
    layout->setContentsMargins(60, 60, 60, 60);

    QLabel* titleLabel = new QLabel("SSH Client", m_welcomeWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(48);
    titleFont.setWeight(QFont::Light);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    titleLabel->setStyleSheet("color: #FFFFFF;");

    QLabel* subtitleLabel = new QLabel("Secure Shell Terminal", m_welcomeWidget);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(14);
    subtitleFont.setWeight(QFont::Light);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    subtitleLabel->setStyleSheet("color: #858585;");

    // Add a clean button with animations
    QPushButton* newConnBtn = new QPushButton("New Connection", m_welcomeWidget);
    newConnBtn->setMinimumSize(200, 45);
    newConnBtn->setMaximumSize(300, 50);
    newConnBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    newConnBtn->setCursor(Qt::PointingHandCursor);
    newConnBtn->setStyleSheet(R"(
        QPushButton {
            background: #007ACC;
            color: #FFFFFF;
            font-size: 14px;
            font-weight: 500;
            padding: 12px 24px;
            border: none;
            border-radius: 2px;
        }
        QPushButton:hover {
            background: #1C97EA;
        }
        QPushButton:pressed {
            background: #005A9E;
        }
    )");

    // Connect button click
    connect(newConnBtn, &QPushButton::clicked, this, &MainWindow::onNewConnection);

    // Saved connections list
    QLabel* savedLabel = new QLabel("Saved Connections", m_welcomeWidget);
    QFont savedFont = savedLabel->font();
    savedFont.setPointSize(12);
    savedFont.setWeight(QFont::Normal);
    savedLabel->setFont(savedFont);
    savedLabel->setAlignment(Qt::AlignLeft);
    savedLabel->setStyleSheet("color: #CCCCCC; padding-left: 10px;");
    savedLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_savedConnectionsList = new QListWidget(m_welcomeWidget);
    m_savedConnectionsList->setMaximumHeight(200);
    m_savedConnectionsList->setMinimumWidth(400);
    m_savedConnectionsList->setMaximumWidth(600);
    m_savedConnectionsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_savedConnectionsList->setStyleSheet(R"(
        QListWidget {
            background: #252526;
            color: #CCCCCC;
            border: 1px solid #3C3C3C;
            border-radius: 3px;
            padding: 8px;
            font-size: 13px;
        }
        QListWidget::item {
            padding: 10px;
            border-radius: 2px;
            margin: 2px 0px;
        }
        QListWidget::item:hover {
            background: #2A2D2E;
        }
        QListWidget::item:selected {
            background: #094771;
            color: #FFFFFF;
        }
    )");

    // Connect list item click
    connect(m_savedConnectionsList, &QListWidget::itemClicked,
            this, &MainWindow::onSavedConnectionClicked);

    // Load saved profiles
    loadSavedProfiles();

    layout->addStretch(2);
    layout->addWidget(titleLabel);
    layout->addSpacing(8);
    layout->addWidget(subtitleLabel);
    layout->addSpacing(50);
    layout->addWidget(newConnBtn, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(savedLabel);
    layout->addSpacing(10);
    layout->addWidget(m_savedConnectionsList, 0, Qt::AlignCenter);
    layout->addStretch(1);

    int index = m_tabWidget->addTab(m_welcomeWidget, "Welcome");

    // Make welcome tab non-closable by setting tab button to nullptr
    m_tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, nullptr);
    m_tabWidget->tabBar()->setTabButton(index, QTabBar::LeftSide, nullptr);

    // Animation disabled for stability
}

void MainWindow::hideWelcomeTab()
{
    if (!m_welcomeWidget) {
        return;
    }

    int index = m_tabWidget->indexOf(m_welcomeWidget);
    if (index >= 0) {
        m_tabWidget->removeTab(index);
    }

    delete m_welcomeWidget;
    m_welcomeWidget = nullptr;
    m_savedConnectionsList = nullptr;
}

void MainWindow::loadSavedProfiles()
{
    qDebug(ui) << "=== loadSavedProfiles START ===";

    if (!m_savedConnectionsList) {
        qWarning(ui) << "m_savedConnectionsList is null, returning";
        return;
    }

    m_savedConnectionsList->clear();

    // Use JSON file instead of QSettings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString filePath = configPath + "/connections.json";

    qDebug(ui) << "Loading profiles from:" << filePath;

    QFile file(filePath);
    if (!file.exists()) {
        qDebug(ui) << "No saved profiles file";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning(ui) << "Failed to open profiles file";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        qWarning(ui) << "Invalid profiles file format";
        return;
    }

    QJsonArray profiles = doc.array();
    qDebug(ui) << "Found" << profiles.size() << "profiles";

    for (const QJsonValue& value : profiles) {
        QJsonObject profile = value.toObject();

        QString name = profile["name"].toString();
        QString host = profile["host"].toString();
        QString user = profile["user"].toString();
        int port = profile["port"].toInt(22);

        if (name.isEmpty() || host.isEmpty()) {
            continue;
        }

        QString displayText = QString("%1 (%2@%3:%4)")
                                  .arg(name)
                                  .arg(user)
                                  .arg(host)
                                  .arg(port);

        QListWidgetItem* item = new QListWidgetItem(displayText);
        QVariantMap profileData;
        profileData["name"] = name;
        profileData["host"] = host;
        profileData["user"] = user;
        profileData["port"] = port;
        profileData["authMethod"] = profile["authMethod"].toInt(0);
        profileData["keyFile"] = profile["keyFile"].toString("");
        item->setData(Qt::UserRole, profileData);
        m_savedConnectionsList->addItem(item);
    }

    qDebug(ui) << "=== loadSavedProfiles END ===" << m_savedConnectionsList->count() << "items";
}

void MainWindow::saveCurrentProfile(const ConnectionProfile& profile)
{
    // Use JSON file instead of QSettings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = configPath + "/connections.json";
    qDebug(ui) << "Saving profile to:" << filePath;

    // Load existing profiles
    QJsonArray profiles;
    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            profiles = doc.array();
        }
    }

    // Check if profile already exists and update it
    bool found = false;
    for (int i = 0; i < profiles.size(); ++i) {
        QJsonObject obj = profiles[i].toObject();
        if (obj["name"].toString() == profile.profileName() &&
            obj["host"].toString() == profile.hostname()) {
            // Update existing profile
            obj["user"] = profile.username();
            obj["port"] = profile.port();
            obj["authMethod"] = static_cast<int>(profile.authMethod());
            obj["keyFile"] = profile.keyFilePath();
            profiles[i] = obj;
            found = true;
            break;
        }
    }

    // Add new profile if not found
    if (!found) {
        QJsonObject newProfile;
        newProfile["name"] = profile.profileName();
        newProfile["host"] = profile.hostname();
        newProfile["user"] = profile.username();
        newProfile["port"] = profile.port();
        newProfile["authMethod"] = static_cast<int>(profile.authMethod());
        newProfile["keyFile"] = profile.keyFilePath();
        profiles.append(newProfile);
    }

    // Save to file
    QJsonDocument doc(profiles);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        qInfo(ui) << "Profile saved:" << profile.profileName();
    } else {
        qWarning(ui) << "Failed to save profile";
    }

    // Add to recent connections
    AppSettings::instance()->addRecentConnection(profile.profileName());
    AppSettings::instance()->save();

    // Reload saved profiles list (only if welcome widget exists)
    if (m_savedConnectionsList) {
        loadSavedProfiles();
    }
}

void MainWindow::onSavedConnectionClicked(QListWidgetItem* item)
{
    qDebug(ui) << "=== onSavedConnectionClicked START ===";

    if (!item) {
        qWarning(ui) << "Item is null";
        return;
    }

    // Check if item is enabled (skip "No saved connections" placeholder)
    if (!(item->flags() & Qt::ItemIsEnabled)) {
        qDebug(ui) << "Item is disabled";
        return;
    }

    // Get stored profile data
    QVariantMap profileData = item->data(Qt::UserRole).toMap();
    if (profileData.isEmpty()) {
        qWarning(ui) << "Profile data is empty";
        return;
    }

    QString user = profileData["user"].toString();
    QString host = profileData["host"].toString();
    int port = profileData["port"].toInt();
    QString name = profileData["name"].toString();

    qDebug(ui) << "Connecting to:" << name << user << host << port;

    if (user.isEmpty() || host.isEmpty()) {
        qWarning(ui) << "User or host is empty";
        return;
    }

    // Create connection profile
    ConnectionProfile profile;
    profile.setProfileName(name);
    profile.setHostname(host);
    profile.setPort(port);
    profile.setUsername(user);
    profile.setAuthMethod(static_cast<ConnectionProfile::AuthMethod>(profileData["authMethod"].toInt()));
    profile.setKeyFilePath(profileData["keyFile"].toString());

    QString password;

    // Try to retrieve password from Keychain
    if (profile.authMethod() == ConnectionProfile::AuthMethod::Password) {
        qDebug(ui) << "Attempting to retrieve password from Keychain";

        CredentialManager* credManager = new CredentialManager();
        QString service = QString("SSH:%1").arg(host);

        try {
            password = credManager->retrievePassword(service, user);
            qDebug(ui) << "Password retrieved, empty:" << password.isEmpty();
        } catch (...) {
            qWarning(ui) << "Exception retrieving password from Keychain";
            password = "";
        }

        if (password.isEmpty()) {
            qDebug(ui) << "No stored password, showing password dialog";
            // No stored password, ask user
            SimplePasswordDialog dialog(user, host, this);
            if (dialog.exec() != QDialog::Accepted) {
                qDebug(ui) << "Password dialog cancelled";
                delete credManager;
                return;
            }
            password = dialog.password();
            if (password.isEmpty()) {
                qWarning(ui) << "Password is empty";
                delete credManager;
                return;
            }

            // Store password for next time
            try {
                credManager->storePassword(service, user, password);
                qInfo(ui) << "Password stored in Keychain";
            } catch (...) {
                qWarning(ui) << "Exception storing password in Keychain";
            }
        } else {
            qInfo(ui) << "Using stored password from Keychain";
        }

        delete credManager;
    }

    qDebug(ui) << "Calling handleConnectionRequest";
    // Connect with password
    handleConnectionRequest(profile, password);
    qDebug(ui) << "=== onSavedConnectionClicked END ===";
}
