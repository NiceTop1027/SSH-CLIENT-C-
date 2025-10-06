#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>
#include "CustomTabBar.h"

class CustomTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit CustomTabWidget(QWidget* parent = nullptr);
    ~CustomTabWidget();

signals:
    void tabDroppedOnTab(int draggedIndex, int targetIndex);

private:
    CustomTabBar* m_customTabBar;
};

#endif // CUSTOMTABWIDGET_H
