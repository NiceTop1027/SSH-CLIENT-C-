#include "CustomTabBar.h"
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QPainter>
#include <QPixmap>

CustomTabBar::CustomTabBar(QWidget* parent)
    : QTabBar(parent)
    , m_draggedTabIndex(-1)
{
    setAcceptDrops(true);
    setMouseTracking(true);
}

CustomTabBar::~CustomTabBar()
{
}

void CustomTabBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        m_draggedTabIndex = tabAt(event->pos());
    }
    QTabBar::mousePressEvent(event);
}

void CustomTabBar::mouseMoveEvent(QMouseEvent* event)
{
    // Only start drag if moved far enough
    if (!(event->buttons() & Qt::LeftButton)) {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    if (m_draggedTabIndex < 0) {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    // Create drag
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();

    // Store tab index in mime data
    mimeData->setText(QString::number(m_draggedTabIndex));
    mimeData->setData("application/x-tabindex", QString::number(m_draggedTabIndex).toUtf8());
    drag->setMimeData(mimeData);

    // Set drag pixmap to tab text
    QString tabText = this->tabText(m_draggedTabIndex);
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QColor(100, 181, 246));
    painter.setFont(font());
    painter.drawText(pixmap.rect(), Qt::AlignCenter, tabText);
    drag->setPixmap(pixmap);

    drag->exec(Qt::MoveAction);

    QTabBar::mouseMoveEvent(event);
}

void CustomTabBar::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-tabindex")) {
        event->acceptProposedAction();
    } else {
        QTabBar::dragEnterEvent(event);
    }
}

void CustomTabBar::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasFormat("application/x-tabindex")) {
        QTabBar::dropEvent(event);
        return;
    }

    int draggedIndex = event->mimeData()->data("application/x-tabindex").toInt();
    int targetIndex = tabAt(event->position().toPoint());

    if (targetIndex >= 0 && draggedIndex != targetIndex) {
        emit tabDroppedOnTab(draggedIndex, targetIndex);
        event->acceptProposedAction();
    } else {
        QTabBar::dropEvent(event);
    }
}
