#include "SplitContainer.h"
#include "TerminalView.h"
#include <QHBoxLayout>
#include <QFrame>

SplitContainer::SplitContainer(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolbar(nullptr)
    , m_rootSplitter(nullptr)
    , m_activeTerminal(nullptr)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setupToolbar();

    // Create root splitter
    m_rootSplitter = new QSplitter(Qt::Horizontal, this);
    m_rootSplitter->setHandleWidth(3);
    m_rootSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background: #3C3C3C;
        }
        QSplitter::handle:hover {
            background: #007ACC;
        }
    )");

    m_mainLayout->addWidget(m_rootSplitter);
}

SplitContainer::~SplitContainer()
{
}

void SplitContainer::setupToolbar()
{
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(36);
    m_toolbar->setStyleSheet("background: #252526; border-bottom: 1px solid #3C3C3C;");

    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolbar);
    toolbarLayout->setContentsMargins(8, 4, 8, 4);
    toolbarLayout->setSpacing(4);

    // Split horizontal button
    QToolButton* splitHBtn = new QToolButton();
    splitHBtn->setText("⬌");
    splitHBtn->setToolTip("Split Horizontal (⌘+D)");
    splitHBtn->setShortcut(QKeySequence("Ctrl+D"));
    splitHBtn->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            color: #CCCCCC;
            border: none;
            padding: 6px 12px;
            font-size: 16px;
        }
        QToolButton:hover {
            background: #37373D;
        }
        QToolButton:pressed {
            background: #094771;
        }
    )");
    connect(splitHBtn, &QToolButton::clicked, this, &SplitContainer::splitHorizontal);

    // Split vertical button
    QToolButton* splitVBtn = new QToolButton();
    splitVBtn->setText("⬍");
    splitVBtn->setToolTip("Split Vertical (⌘+Shift+D)");
    splitVBtn->setShortcut(QKeySequence("Ctrl+Shift+D"));
    splitVBtn->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            color: #CCCCCC;
            border: none;
            padding: 6px 12px;
            font-size: 16px;
        }
        QToolButton:hover {
            background: #37373D;
        }
        QToolButton:pressed {
            background: #094771;
        }
    )");
    connect(splitVBtn, &QToolButton::clicked, this, &SplitContainer::splitVertical);

    toolbarLayout->addWidget(splitHBtn);
    toolbarLayout->addWidget(splitVBtn);
    toolbarLayout->addStretch();

    m_mainLayout->addWidget(m_toolbar);
}

void SplitContainer::addTerminal(TerminalView* terminal)
{
    if (!terminal) {
        return;
    }

    createSplitWidget(terminal);
    m_terminals.append(terminal);
    m_activeTerminal = terminal;

    emit terminalAdded(terminal);
    emit activeTerminalChanged(terminal);
}

void SplitContainer::createSplitWidget(TerminalView* terminal)
{
    // Create a frame for the terminal with border highlight
    QFrame* frame = new QFrame();
    frame->setFrameStyle(QFrame::NoFrame);
    frame->setStyleSheet(R"(
        QFrame {
            border: 2px solid transparent;
        }
        QFrame:focus {
            border: 2px solid #007ACC;
        }
    )");

    QVBoxLayout* frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(terminal);

    m_rootSplitter->addWidget(frame);

    // Focus handling
    terminal->installEventFilter(this);
    connect(terminal, &TerminalView::sendData, this, [this, terminal]() {
        m_activeTerminal = terminal;
        emit activeTerminalChanged(terminal);
    });
}

void SplitContainer::splitHorizontal()
{
    if (!m_activeTerminal) {
        return;
    }

    // Create new terminal (caller will set up the connection)
    TerminalView* newTerminal = new TerminalView();

    // Find parent splitter of active terminal
    QWidget* activeWidget = m_activeTerminal->parentWidget();
    QSplitter* parentSplitter = qobject_cast<QSplitter*>(activeWidget->parentWidget());

    if (!parentSplitter) {
        // No parent splitter, create one
        int index = m_rootSplitter->indexOf(activeWidget);
        m_rootSplitter->widget(index)->setParent(nullptr);

        QSplitter* newSplitter = new QSplitter(Qt::Horizontal);
        newSplitter->setHandleWidth(3);
        newSplitter->setStyleSheet(m_rootSplitter->styleSheet());

        QFrame* frame = qobject_cast<QFrame*>(activeWidget);
        newSplitter->addWidget(frame);

        QFrame* newFrame = new QFrame();
        newFrame->setStyleSheet(frame->styleSheet());
        QVBoxLayout* newLayout = new QVBoxLayout(newFrame);
        newLayout->setContentsMargins(0, 0, 0, 0);
        newLayout->addWidget(newTerminal);
        newSplitter->addWidget(newFrame);

        m_rootSplitter->insertWidget(index, newSplitter);
    } else {
        // Add to existing splitter
        createSplitWidget(newTerminal);
    }

    m_terminals.append(newTerminal);
    m_activeTerminal = newTerminal;

    emit terminalAdded(newTerminal);
    emit activeTerminalChanged(newTerminal);
}

void SplitContainer::splitVertical()
{
    if (!m_activeTerminal) {
        return;
    }

    TerminalView* newTerminal = new TerminalView();

    QWidget* activeWidget = m_activeTerminal->parentWidget();
    QSplitter* parentSplitter = qobject_cast<QSplitter*>(activeWidget->parentWidget());

    if (!parentSplitter || parentSplitter->orientation() != Qt::Vertical) {
        // Create vertical splitter
        int index = m_rootSplitter->indexOf(activeWidget);
        if (index < 0 && parentSplitter) {
            index = parentSplitter->indexOf(activeWidget);
            parentSplitter = qobject_cast<QSplitter*>(parentSplitter->parentWidget());
        }

        if (parentSplitter) {
            parentSplitter->widget(index)->setParent(nullptr);

            QSplitter* newSplitter = new QSplitter(Qt::Vertical);
            newSplitter->setHandleWidth(3);
            newSplitter->setStyleSheet(m_rootSplitter->styleSheet());

            QFrame* frame = qobject_cast<QFrame*>(activeWidget);
            newSplitter->addWidget(frame);

            QFrame* newFrame = new QFrame();
            newFrame->setStyleSheet(frame->styleSheet());
            QVBoxLayout* newLayout = new QVBoxLayout(newFrame);
            newLayout->setContentsMargins(0, 0, 0, 0);
            newLayout->addWidget(newTerminal);
            newSplitter->addWidget(newFrame);

            parentSplitter->insertWidget(index, newSplitter);
        }
    } else {
        createSplitWidget(newTerminal);
    }

    m_terminals.append(newTerminal);
    m_activeTerminal = newTerminal;

    emit terminalAdded(newTerminal);
    emit activeTerminalChanged(newTerminal);
}

TerminalView* SplitContainer::currentTerminal() const
{
    return m_activeTerminal;
}

QList<TerminalView*> SplitContainer::terminals() const
{
    return m_terminals;
}
