#ifndef SPLITVIEW_H
#define SPLITVIEW_H

#include <QWidget>
#include <QSplitter>
#include <QList>

class TerminalView;

class SplitView : public QWidget {
    Q_OBJECT

public:
    explicit SplitView(QWidget* parent = nullptr);
    ~SplitView();

    // Split operations
    void splitHorizontally();
    void splitVertically();

    // Terminal access
    TerminalView* activeTerminal() const;
    QList<TerminalView*> allTerminals() const;

    // Initial terminal
    TerminalView* initialTerminal() const { return m_initialTerminal; }

signals:
    void terminalAdded(TerminalView* terminal);
    void activeTerminalChanged(TerminalView* terminal);

private:
    void setupUi();
    QWidget* createSplitterContainer(QWidget* widget1, QWidget* widget2, Qt::Orientation orientation);

    TerminalView* m_initialTerminal;
    TerminalView* m_activeTerminal;
    QWidget* m_rootContainer;
};

#endif // SPLITVIEW_H
