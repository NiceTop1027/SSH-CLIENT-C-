#include "SplitView.h"
#include "TerminalView.h"
#include <QVBoxLayout>
#include <QSplitter>

SplitView::SplitView(QWidget* parent)
    : QWidget(parent)
    , m_initialTerminal(nullptr)
    , m_activeTerminal(nullptr)
    , m_rootContainer(nullptr)
{
    setupUi();
}

SplitView::~SplitView()
{
}

void SplitView::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create initial terminal
    m_initialTerminal = new TerminalView(this);
    m_activeTerminal = m_initialTerminal;
    m_rootContainer = m_initialTerminal;

    layout->addWidget(m_rootContainer);

    // Set focus to initial terminal
    m_initialTerminal->setFocus();
}

void SplitView::splitHorizontally()
{
    if (!m_activeTerminal) {
        return;
    }

    // Create new terminal
    TerminalView* newTerminal = new TerminalView(this);

    // Create splitter container
    QWidget* newContainer = createSplitterContainer(m_activeTerminal, newTerminal, Qt::Vertical);

    // Replace active terminal with splitter in the layout
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        // Find and replace the old container
        QWidget* oldContainer = m_rootContainer;
        mainLayout->removeWidget(oldContainer);

        if (oldContainer != m_activeTerminal) {
            oldContainer->deleteLater();
        }

        m_rootContainer = newContainer;
        mainLayout->addWidget(m_rootContainer);
    }

    m_activeTerminal = newTerminal;
    newTerminal->setFocus();

    emit terminalAdded(newTerminal);
    emit activeTerminalChanged(newTerminal);
}

void SplitView::splitVertically()
{
    if (!m_activeTerminal) {
        return;
    }

    // Create new terminal
    TerminalView* newTerminal = new TerminalView(this);

    // Create splitter container
    QWidget* newContainer = createSplitterContainer(m_activeTerminal, newTerminal, Qt::Horizontal);

    // Replace active terminal with splitter in the layout
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        // Find and replace the old container
        QWidget* oldContainer = m_rootContainer;
        mainLayout->removeWidget(oldContainer);

        if (oldContainer != m_activeTerminal) {
            oldContainer->deleteLater();
        }

        m_rootContainer = newContainer;
        mainLayout->addWidget(m_rootContainer);
    }

    m_activeTerminal = newTerminal;
    newTerminal->setFocus();

    emit terminalAdded(newTerminal);
    emit activeTerminalChanged(newTerminal);
}

QWidget* SplitView::createSplitterContainer(QWidget* widget1, QWidget* widget2, Qt::Orientation orientation)
{
    QSplitter* splitter = new QSplitter(orientation, this);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet(R"(
        QSplitter::handle {
            background: #3A3A3A;
        }
        QSplitter::handle:hover {
            background: #64B5F6;
        }
    )");

    // Reparent widgets to splitter
    widget1->setParent(splitter);
    widget2->setParent(splitter);

    splitter->addWidget(widget1);
    splitter->addWidget(widget2);

    // Equal sizes
    splitter->setSizes(QList<int>() << 1 << 1);

    return splitter;
}

TerminalView* SplitView::activeTerminal() const
{
    return m_activeTerminal;
}

QList<TerminalView*> SplitView::allTerminals() const
{
    QList<TerminalView*> terminals;

    // Find all TerminalView widgets recursively
    QList<TerminalView*> allChildren = findChildren<TerminalView*>();
    for (TerminalView* terminal : allChildren) {
        terminals.append(terminal);
    }

    return terminals;
}
