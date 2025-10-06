#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ConnectionProfile.h"
#include "TerminalView.h"
// #include "ProfileStorage.h"  // Not used anymore, using JSON files directly
#include "SplitContainer.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QAction>
#include <QListWidget>

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

    // UI access
    QTabWidget* tabWidget() const;

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

    // Connection handling
    void handleConnectionRequest(const ConnectionProfile& profile, const QString& password = QString());
    void handleConnected(int tabIndex);
    void handleDisconnected();
    void handleError(const QString& error);
    void handleDataReceived(int tabIndex, const QByteArray& data);

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
    QTabWidget* m_tabWidget;
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

    // Connection management
    struct TabData {
        SplitContainer* splitContainer;
        TerminalView* terminal;
        SSHConnection* connection;
        SSHWorkerThread* worker;
    };

    QList<TabData> m_tabs;
    // ProfileStorage* m_profileStorage;  // Not used anymore, using JSON files directly
};

#endif // MAINWINDOW_H
