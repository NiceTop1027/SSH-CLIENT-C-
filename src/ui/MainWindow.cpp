#include "MainWindow.h"
#include "ConnectionDialog.h"
#include "PreferencesDialog.h"
#include "AppSettings.h"
#include "SSHConnection.h"
#include "SSHAuthenticator.h"
#include "SSHWorkerThread.h"
#include "Logger.h"
#include "ErrorDialog.h"
#include "TerminalView.h"
#include "SplitView.h"
#include "CustomTabWidget.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTabBar>
#include <QWidgetAction>

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

    // Load window size from settings
    AppSettings* settings = AppSettings::instance();
    if (settings->rememberWindowSize()) {
        resize(settings->lastWindowSize());
    } else {
        resize(1024, 768);
    }
    setMinimumSize(800, 600);

    // Apply elegant dark theme
    setStyleSheet(R"(
        QMainWindow {
            background: #1A1A1A;
        }
        QMenuBar {
            background: #252525;
            color: #E8E8E8;
            border-bottom: 1px solid #3A3A3A;
            padding: 6px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 8px 16px;
            border-radius: 6px;
            margin: 2px;
        }
        QMenuBar::item:selected {
            background: #333333;
        }
        QMenu {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 10px;
            padding: 6px;
        }
        QMenu::item {
            padding: 10px 28px;
            border-radius: 6px;
            margin: 2px 4px;
        }
        QMenu::item:selected {
            background: #3A3A3A;
        }
        QTabWidget::pane {
            border: none;
            background: #1A1A1A;
        }
        QTabBar::tab {
            background: transparent;
            color: #9E9E9E;
            padding: 12px 24px;
            margin-right: 2px;
            border: none;
            border-bottom: 2px solid transparent;
            font-size: 13px;
        }
        QTabBar::tab:selected {
            color: #64B5F6;
            border-bottom: 2px solid #64B5F6;
            font-weight: 500;
        }
        QTabBar::tab:hover {
            background: #252525;
            color: #BDBDBD;
        }
        QStatusBar {
            background: #252525;
            color: #9E9E9E;
            border-top: 1px solid #3A3A3A;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #42A5F5, stop:1 #1E88E5);
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: 500;
            font-size: 13px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #64B5F6, stop:1 #42A5F5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #1976D2, stop:1 #1565C0);
        }
        QLineEdit {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 6px;
            padding: 8px 14px;
            selection-background-color: #1976D2;
        }
        QLineEdit:focus {
            border: 1px solid #64B5F6;
            background: #2F2F2F;
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

    delete m_profileStorage;
    qInfo(ui) << "MainWindow cleanup complete";
}

void MainWindow::addNewTab(const QString& title)
{
    TabData tabData;
    tabData.splitView = new SplitView();
    tabData.terminal = tabData.splitView->initialTerminal();

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

    int index = m_tabWidget->addTab(tabData.splitView, title);
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

CustomTabWidget* MainWindow::tabWidget() const
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

void MainWindow::splitHorizontally()
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        m_tabs[index].splitView->splitHorizontally();
    }
}

void MainWindow::splitVertically()
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        m_tabs[index].splitView->splitVertically();
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

void MainWindow::onTabDroppedOnTab(int draggedIndex, int targetIndex)
{
    if (draggedIndex < 0 || targetIndex < 0 ||
        draggedIndex >= m_tabs.size() || targetIndex >= m_tabs.size() ||
        draggedIndex == targetIndex) {
        return;
    }

    qInfo(ui) << "Tab" << draggedIndex << "dropped on tab" << targetIndex;

    // Get the target tab's split view
    SplitView* targetSplitView = m_tabs[targetIndex].splitView;

    // Split the target vertically (VSCode-style)
    targetSplitView->splitVertically();

    // Get the newly created terminal from split
    QList<TerminalView*> allTerminals = targetSplitView->allTerminals();
    if (allTerminals.size() >= 2) {
        TerminalView* newTerminal = allTerminals.last();

        // Move the connection from dragged tab to the new split terminal
        // TabData& draggedTab = m_tabs[draggedIndex]; // Unused for now

        // If dragged tab has a connection, we need to create a new one
        // For now, just show a message that connection should be established
        // In a full implementation, you'd clone the connection or create a new session

        // Copy terminal settings
        AppSettings* settings = AppSettings::instance();
        newTerminal->setBackgroundImage(settings->terminalBackgroundImage());
        newTerminal->setBackgroundImageOpacity(settings->backgroundImageOpacity());
        QFont font(settings->terminalFontFamily(), settings->terminalFontSize());
        newTerminal->setCustomFont(font);
        newTerminal->setCustomColors(settings->terminalForegroundColor(),
                                     settings->terminalBackgroundColor());

        // Display welcome message in new split
        QString tabName = m_tabWidget->tabText(draggedIndex);
        newTerminal->displayOutput(QString("Split from: %1\n").arg(tabName));
        newTerminal->displayOutput(QString("Ready for new connection.\n"));
    }

    // Close the original dragged tab
    onTabCloseRequested(draggedIndex);
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
    m_tabWidget = new CustomTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setAcceptDrops(true);

    // Connect drag-drop signal
    connect(m_tabWidget, &CustomTabWidget::tabDroppedOnTab, this, &MainWindow::onTabDroppedOnTab);

    // Add "New Connection" button with dropdown to tab bar
    QPushButton* newTabButton = new QPushButton("+");
    newTabButton->setFixedSize(28, 28);
    newTabButton->setStyleSheet(R"(
        QPushButton {
            background: #2A2A2A;
            color: #64B5F6;
            border: 1px solid #3A3A3A;
            border-radius: 6px;
            font-size: 18px;
            font-weight: bold;
            padding-bottom: 2px;
        }
        QPushButton:hover {
            background: #3A3A3A;
            border-color: #64B5F6;
        }
        QPushButton:pressed {
            background: #333333;
        }
        QPushButton::menu-indicator {
            image: none;
        }
    )");
    newTabButton->setToolTip("New Connection");

    // Create menu for new connection button
    QMenu* newConnMenu = new QMenu(newTabButton);
    newConnMenu->setStyleSheet(R"(
        QMenu {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 8px;
            padding: 6px;
        }
        QMenu::item {
            padding: 10px 28px;
            border-radius: 6px;
            margin: 2px 4px;
        }
        QMenu::item:selected {
            background: #3A3A3A;
        }
        QMenu::separator {
            height: 1px;
            background: #3A3A3A;
            margin: 6px 10px;
        }
    )");

    // Update menu dynamically when button is clicked
    connect(newTabButton, &QPushButton::clicked, this, [this, newTabButton, newConnMenu]() {
        // Clear existing actions
        newConnMenu->clear();

        // Add "New Connection" option
        QAction* newConnAction = newConnMenu->addAction("New Connection...");
        newConnAction->setShortcut(QKeySequence::New);
        connect(newConnAction, &QAction::triggered, this, &MainWindow::onNewConnection);

        // Add separator
        newConnMenu->addSeparator();

        // Add saved connections with delete buttons
        QList<ConnectionProfile> profiles = m_profileStorage->loadAllProfiles();
        if (!profiles.isEmpty()) {
            for (const ConnectionProfile& profile : profiles) {
                QString label = QString("%1@%2:%3")
                    .arg(profile.username())
                    .arg(profile.hostname())
                    .arg(profile.port());

                // Create custom widget with label and delete button
                QWidget* itemWidget = new QWidget(newConnMenu);
                itemWidget->setMinimumWidth(250);
                itemWidget->setMinimumHeight(32);

                QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
                itemLayout->setContentsMargins(12, 6, 12, 6);
                itemLayout->setSpacing(12);

                // Connection label (clickable area)
                QPushButton* nameBtn = new QPushButton(label, itemWidget);
                nameBtn->setFlat(true);
                nameBtn->setCursor(Qt::PointingHandCursor);
                nameBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                nameBtn->setStyleSheet(R"(
                    QPushButton {
                        background: transparent;
                        color: #E8E8E8;
                        border: none;
                        text-align: left;
                        padding: 6px;
                        min-height: 24px;
                    }
                    QPushButton:hover {
                        background: #3A3A3A;
                        border-radius: 4px;
                    }
                )");
                connect(nameBtn, &QPushButton::clicked, newConnMenu, [this, profile, newConnMenu]() {
                    newConnMenu->close();

                    // Show connection dialog with pre-filled profile
                    ConnectionDialog dialog(this);
                    dialog.setProfile(profile);

                    if (dialog.exec() == QDialog::Accepted) {
                        ConnectionProfile updatedProfile = dialog.getProfile();
                        QString password = dialog.getPassword();
                        handleConnectionRequest(updatedProfile, password);
                    }
                });
                itemLayout->addWidget(nameBtn, 1);

                // Delete button
                QPushButton* deleteBtn = new QPushButton("✕", itemWidget);
                deleteBtn->setFixedSize(24, 24);
                deleteBtn->setCursor(Qt::PointingHandCursor);
                deleteBtn->setStyleSheet(R"(
                    QPushButton {
                        background: #3A3A3A;
                        color: #9E9E9E;
                        border: none;
                        border-radius: 4px;
                        font-size: 16px;
                        font-weight: bold;
                        padding: 0px;
                    }
                    QPushButton:hover {
                        background: #E53935;
                        color: white;
                    }
                )");
                deleteBtn->setToolTip("Delete this connection");

                connect(deleteBtn, &QPushButton::clicked, this, [this, profile, newConnMenu]() {
                    // Delete the profile
                    m_profileStorage->deleteProfile(profile.profileName());

                    // Close menu
                    newConnMenu->close();

                    // Show status message
                    showStatusMessage(QString("Deleted: %1").arg(profile.profileName()), 2000);
                });

                itemLayout->addWidget(deleteBtn);

                // Set widget action
                QWidgetAction* widgetAction = new QWidgetAction(newConnMenu);
                widgetAction->setDefaultWidget(itemWidget);
                newConnMenu->addAction(widgetAction);
            }
        } else {
            QAction* noSavedAction = newConnMenu->addAction("No saved connections");
            noSavedAction->setEnabled(false);
        }

        newTabButton->showMenu();
    });

    m_tabWidget->setCornerWidget(newTabButton, Qt::TopRightCorner);

    // Set size policy to expand with window
    m_tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
    fileMenu->addAction(m_preferencesAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->setObjectName("editMenu");
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->setObjectName("viewMenu");
    viewMenu->addAction(m_splitHorizontalAction);
    viewMenu->addAction(m_splitVerticalAction);

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

    m_splitHorizontalAction = new QAction("Split &Horizontally", this);
    m_splitHorizontalAction->setObjectName("splitHorizontalAction");
    m_splitHorizontalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    connect(m_splitHorizontalAction, &QAction::triggered, this, &MainWindow::splitHorizontally);

    m_splitVerticalAction = new QAction("Split &Vertically", this);
    m_splitVerticalAction->setObjectName("splitVerticalAction");
    m_splitVerticalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(m_splitVerticalAction, &QAction::triggered, this, &MainWindow::splitVertically);
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

    QVBoxLayout* layout = new QVBoxLayout(m_welcomeWidget);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("SSH Client", m_welcomeWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(42);
    titleFont.setWeight(QFont::Light);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    titleLabel->setStyleSheet("color: #E8E8E8; letter-spacing: 2px;");

    QLabel* infoLabel = new QLabel("Secure Shell Terminal", m_welcomeWidget);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: #9E9E9E; font-size: 16px; font-weight: 300;");
    infoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    infoLabel->setWordWrap(true);

    // Add a button for quick access
    QPushButton* newConnBtn = new QPushButton("New Connection", m_welcomeWidget);
    newConnBtn->setMinimumSize(200, 52);
    newConnBtn->setMaximumSize(220, 52);
    newConnBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    newConnBtn->setCursor(Qt::PointingHandCursor);
    newConnBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #42A5F5, stop:1 #1E88E5);
            color: #FFFFFF;
            border: none;
            border-radius: 26px;
            padding: 14px 36px;
            font-size: 15px;
            font-weight: 500;
            letter-spacing: 0.5px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #64B5F6, stop:1 #42A5F5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #1976D2, stop:1 #1565C0);
        }
    )");
    connect(newConnBtn, &QPushButton::clicked, this, &MainWindow::onNewConnection);

    // Saved connections section
    QLabel* savedLabel = new QLabel("Recent Connections", m_welcomeWidget);
    savedLabel->setStyleSheet("font-size: 14px; font-weight: 500; color: #BDBDBD; letter-spacing: 0.5px;");
    savedLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_savedConnectionsList = new QListWidget(m_welcomeWidget);
    m_savedConnectionsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_savedConnectionsList->setMinimumHeight(280);
    m_savedConnectionsList->setMinimumWidth(600);
    m_savedConnectionsList->setMaximumWidth(900);
    m_savedConnectionsList->setStyleSheet(R"(
        QListWidget {
            background: #252525;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 10px;
            padding: 12px;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 16px 20px;
            border-radius: 6px;
            margin: 4px 0px;
            background: transparent;
        }
        QListWidget::item:hover {
            background: #2F2F2F;
        }
        QListWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                       stop:0 rgba(66, 165, 245, 0.2),
                                       stop:1 rgba(30, 136, 229, 0.2));
            color: #64B5F6;
            border-left: 3px solid #64B5F6;
            padding-left: 17px;
        }
        QListWidget::item:!selected {
            color: #E8E8E8;
        }
    )");
    connect(m_savedConnectionsList, &QListWidget::itemDoubleClicked, this, &MainWindow::onSavedConnectionClicked);

    // Load saved profiles
    loadSavedProfiles();

    layout->addStretch(2);
    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(infoLabel);
    layout->addSpacing(40);
    layout->addWidget(newConnBtn, 0, Qt::AlignCenter);
    layout->addSpacing(50);
    layout->addWidget(savedLabel);
    layout->addSpacing(12);
    layout->addWidget(m_savedConnectionsList, 0, Qt::AlignCenter);
    layout->addStretch(2);

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

        // Add to recent connections
        AppSettings::instance()->addRecentConnection(profile.profileName());
        AppSettings::instance()->save();

        // Reload saved profiles list
        loadSavedProfiles();
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
