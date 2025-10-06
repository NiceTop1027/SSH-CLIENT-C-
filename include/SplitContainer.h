#ifndef SPLITCONTAINER_H
#define SPLITCONTAINER_H

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QToolButton>
#include <QList>

class TerminalView;

class SplitContainer : public QWidget {
    Q_OBJECT

public:
    explicit SplitContainer(QWidget* parent = nullptr);
    ~SplitContainer();

    // Add terminal to the container
    void addTerminal(TerminalView* terminal);

    // Split operations
    void splitHorizontal();
    void splitVertical();

    // Get current active terminal
    TerminalView* currentTerminal() const;

    // Get all terminals
    QList<TerminalView*> terminals() const;

signals:
    void terminalAdded(TerminalView* terminal);
    void terminalRemoved(TerminalView* terminal);
    void activeTerminalChanged(TerminalView* terminal);

private:
    void setupToolbar();
    void createSplitWidget(TerminalView* terminal);

    QVBoxLayout* m_mainLayout;
    QWidget* m_toolbar;
    QSplitter* m_rootSplitter;
    QList<TerminalView*> m_terminals;
    TerminalView* m_activeTerminal;
};

#endif // SPLITCONTAINER_H
