#include "MainWindow.h"
#include "ConnectionDialog.h"
#include "SSHConnection.h"
#include "SSHAuthenticator.h"
#include "SSHWorkerThread.h"
#include "Logger.h"
#include "ErrorDialog.h"
#include "TerminalView.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTabBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_tabWidget(nullptr), m_welcomeWidget(nullptr), m_savedConnectionsList(nullptr)
{
    qInfo(ui) << "MainWindow initializing";

    // Initialize profile storage
    m_profileStorage = new ProfileStorage();

    setupUi();
    createActions();
    createMenus();
    createStatusBar();
    setupConnections();

    // Set window properties
    setWindowTitle("SSH Client");
    resize(1024, 768);
    setMinimumSize(800, 600);

    // Ensure window is visible and raised
    raise();
    activateWindow();

    qInfo(ui) << "MainWindow initialized successfully";
}

MainWindow::~MainWindow()
{
    qInfo(ui) << "MainWindow shutting down, cleaning up" << m_tabs.size() << "connections";
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

    delete m_profileStorage;
    qInfo(ui) << "MainWindow cleanup complete";
}

void MainWindow::addNewTab(const QString& title)
{
    TabData tabData;
    tabData.terminal = new TerminalView();
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
            }
        }

        // Save profile for future use
        saveCurrentProfile(profile);

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
        QClipboard* clipboard = QApplication::clipboard();
        // TODO: Implement text selection in TerminalView
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

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About SSH Client",
                       "SSH Client\n\n"
                       "A cross-platform SSH terminal client built with Qt and libssh.\n\n"
                       "Features:\n"
                       "- Multiple concurrent connections\n"
                       "- Password and key authentication\n"
                       "- Terminal emulation with ANSI support");
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
    // Create new window for each connection
    MainWindow* newWindow = new MainWindow();
    newWindow->show();
    newWindow->raise();
    newWindow->activateWindow();

    // Hide welcome tab in new window
    newWindow->hideWelcomeTab();

    // Create new terminal tab in new window
    newWindow->addNewTab(profile.profileName());

    int index = newWindow->m_tabWidget->currentIndex();
    if (index < 0 || index >= newWindow->m_tabs.size()) {
        return;
    }

    TabData& tabData = newWindow->m_tabs[index];

    // Create connection
    tabData.connection = new SSHConnection(profile);

    // Create authenticator with credentials
    SSHAuthenticator* authenticator = new SSHAuthenticator(profile, password, profile.keyFilePath());
    tabData.connection->setAuthenticator(authenticator);

    // Setup connection signals
    connect(tabData.connection, &SSHConnection::connected, newWindow, &MainWindow::handleConnected);
    connect(tabData.connection, &SSHConnection::disconnected, newWindow,
            &MainWindow::handleDisconnected);
    connect(tabData.connection, &SSHConnection::error, newWindow, &MainWindow::handleError);

    // Connect to host
    newWindow->showStatusMessage("Connecting to " + profile.hostname() + "...");
    tabData.connection->connectToHost();
}

void MainWindow::handleConnected()
{
    showStatusMessage("Connected", 3000);

    int index = m_tabWidget->currentIndex();
    if (index < 0 || index >= m_tabs.size()) {
        return;
    }

    TabData& tabData = m_tabs[index];

    // Create worker thread
    tabData.worker = new SSHWorkerThread(tabData.connection);

    // Connect worker signals
    connect(tabData.worker, &SSHWorkerThread::dataReceived, this,
            &MainWindow::handleDataReceived);
    connect(tabData.worker, &SSHWorkerThread::error, this, &MainWindow::handleError);
    connect(tabData.worker, &SSHWorkerThread::disconnected, this,
            &MainWindow::handleDisconnected);

    // Connect terminal to worker
    connect(tabData.terminal, &TerminalView::sendData,
            [worker = tabData.worker](const QString& data) {
                worker->writeData(data.toUtf8());
            });

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

void MainWindow::handleDataReceived(const QByteArray& data)
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        m_tabs[index].terminal->displayOutput(QString::fromUtf8(data));
    }
}

void MainWindow::setupUi()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this,
            &MainWindow::onTabCloseRequested);

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
    QVBoxLayout* layout = new QVBoxLayout(m_welcomeWidget);

    QLabel* titleLabel = new QLabel("Welcome to SSH Client", m_welcomeWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* infoLabel = new QLabel("Click 'New Connection' or press Ctrl+N to connect to a server", m_welcomeWidget);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: gray; font-size: 14px;");

    // Add a button for quick access
    QPushButton* newConnBtn = new QPushButton("New Connection", m_welcomeWidget);
    newConnBtn->setMinimumSize(200, 50);
    newConnBtn->setStyleSheet("QPushButton { font-size: 16px; padding: 10px; }");
    connect(newConnBtn, &QPushButton::clicked, this, &MainWindow::onNewConnection);

    // Saved connections section
    QLabel* savedLabel = new QLabel("Saved Connections:", m_welcomeWidget);
    savedLabel->setStyleSheet("font-size: 16px; font-weight: bold;");

    m_savedConnectionsList = new QListWidget(m_welcomeWidget);
    m_savedConnectionsList->setMaximumHeight(200);
    m_savedConnectionsList->setStyleSheet("QListWidget { font-size: 14px; }");
    connect(m_savedConnectionsList, &QListWidget::itemDoubleClicked, this, &MainWindow::onSavedConnectionClicked);

    // Load saved profiles
    loadSavedProfiles();

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addSpacing(20);
    layout->addWidget(infoLabel);
    layout->addSpacing(30);
    layout->addWidget(newConnBtn, 0, Qt::AlignCenter);
    layout->addSpacing(30);
    layout->addWidget(savedLabel);
    layout->addSpacing(10);
    layout->addWidget(m_savedConnectionsList);
    layout->addStretch();

    int index = m_tabWidget->addTab(m_welcomeWidget, "Welcome");

    // Make welcome tab non-closable by setting tab button to nullptr
    m_tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, nullptr);
    m_tabWidget->tabBar()->setTabButton(index, QTabBar::LeftSide, nullptr);
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
    if (!m_savedConnectionsList) {
        return;
    }

    m_savedConnectionsList->clear();

    QList<ConnectionProfile> profiles = m_profileStorage->loadAllProfiles();

    if (profiles.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem("No saved connections");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setForeground(Qt::gray);
        m_savedConnectionsList->addItem(item);
        return;
    }

    for (const ConnectionProfile& profile : profiles) {
        QString displayText = QString("%1 (%2@%3:%4)")
                                  .arg(profile.profileName())
                                  .arg(profile.username())
                                  .arg(profile.hostname())
                                  .arg(profile.port());

        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, profile.profileName());
        m_savedConnectionsList->addItem(item);
    }
}

void MainWindow::saveCurrentProfile(const ConnectionProfile& profile)
{
    if (!m_profileStorage) {
        return;
    }

    if (m_profileStorage->saveProfile(profile)) {
        qInfo(ui) << "Profile saved:" << profile.profileName();
    } else {
        qWarning(ui) << "Failed to save profile:" << profile.profileName();
    }
}

void MainWindow::onSavedConnectionClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    QString profileName = item->data(Qt::UserRole).toString();
    if (profileName.isEmpty()) {
        return;
    }

    ConnectionProfile profile = m_profileStorage->loadProfile(profileName);
    if (!profile.isValid()) {
        qWarning(ui) << "Failed to load profile:" << profileName;
        ErrorDialog::showError(this, "Load Error",
                               "Failed to load saved connection: " + profileName,
                               ErrorDialog::ErrorType::Configuration);
        return;
    }

    // For saved profiles, we can't retrieve passwords
    // User will need to enter password if it's password auth
    QString password;
    if (profile.authMethod() == ConnectionProfile::AuthMethod::Password) {
        // Show connection dialog with pre-filled data
        ConnectionDialog dialog(this);
        dialog.setProfile(profile);

        if (dialog.exec() == QDialog::Accepted) {
            QLineEdit* passwordEdit = dialog.findChild<QLineEdit*>("passwordEdit");
            if (passwordEdit) {
                password = passwordEdit->text();
            }
            handleConnectionRequest(profile, password);
        }
    } else {
        // Public key auth - connect directly
        handleConnectionRequest(profile, QString());
    }
}
