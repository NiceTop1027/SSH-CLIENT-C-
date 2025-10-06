#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ConnectionProfile.h"
#include "TerminalView.h"
#include "ProfileStorage.h"
#include "SplitView.h"
#include <QMainWindow>
#include <QAction>
#include <QListWidget>

class CustomTabWidget;

class SSHConnection;
class SSHWorkerThread;
class ConnectionDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Tab management
    void addNewTab(const QString& title);
    void closeCurrentTab();
    TerminalView* currentTerminal();

    // Split management
    void splitHorizontally();
    void splitVertically();

    // UI access
    CustomTabWidget* tabWidget() const;

    // Status
    void showStatusMessage(const QString& message, int timeout = 3000);

signals:
    void connectionRequested(const ConnectionProfile& profile);

private slots:
    void onNewConnection();
    void onCloseTab();
    void onExit();
    void onCopy();
    void onPaste();
    void onPreferences();
    void onAbout();
    void onTabCloseRequested(int index);
    void onSavedConnectionClicked(QListWidgetItem* item);
    void onSettingsChanged();
    void onTabDroppedOnTab(int draggedIndex, int targetIndex);

    // Connection handling
    void handleConnectionRequest(const ConnectionProfile& profile, const QString& password = QString());
    void handleConnected();
    void handleDisconnected();
    void handleError(const QString& error);
    void handleDataReceived(const QByteArray& data);

private:
    void setupUi();
    void createMenus();
    void createActions();
    void createStatusBar();
    void setupConnections();
    void showWelcomeTab();
    void hideWelcomeTab();
    void loadSavedProfiles();
    void saveCurrentProfile(const ConnectionProfile& profile);

    // UI components
    CustomTabWidget* m_tabWidget;
    QWidget* m_welcomeWidget;
    QListWidget* m_savedConnectionsList;

    // Actions
    QAction* m_newConnectionAction;
    QAction* m_closeTabAction;
    QAction* m_exitAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_preferencesAction;
    QAction* m_aboutAction;
    QAction* m_splitHorizontalAction;
    QAction* m_splitVerticalAction;

    // Connection management
    struct TabData {
        SplitView* splitView;
        TerminalView* terminal;
        SSHConnection* connection;
        SSHWorkerThread* worker;
    };

    QList<TabData> m_tabs;
    ProfileStorage* m_profileStorage;
};

#endif // MAINWINDOW_H
