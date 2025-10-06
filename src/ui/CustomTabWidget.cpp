#include "CustomTabWidget.h"

CustomTabWidget::CustomTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    m_customTabBar = new CustomTabBar(this);
    setTabBar(m_customTabBar);

    // Forward signal
    connect(m_customTabBar, &CustomTabBar::tabDroppedOnTab,
            this, &CustomTabWidget::tabDroppedOnTab);
}

CustomTabWidget::~CustomTabWidget()
{
}
