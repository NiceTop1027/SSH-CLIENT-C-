#include "MainWindow.h"
#include "Logger.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QCoreApplication::setOrganizationName("SSH-Client");
    QCoreApplication::setApplicationName("SSH Client");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Initialize logging
    Logger::instance().initialize();
    qInfo(ui) << "Application started - version" << QCoreApplication::applicationVersion();

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
