#ifndef CUSTOMTABBAR_H
#define CUSTOMTABBAR_H

#include <QTabBar>
#include <QDrag>

class CustomTabBar : public QTabBar {
    Q_OBJECT

public:
    explicit CustomTabBar(QWidget* parent = nullptr);
    ~CustomTabBar();

signals:
    void tabDroppedOnTab(int draggedIndex, int targetIndex);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QPoint m_dragStartPos;
    int m_draggedTabIndex;
};

#endif // CUSTOMTABBAR_H
