#include "connection_panel.h"
#include <QPainter>

ConnectionPanel::ConnectionPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void ConnectionPanel::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    update();
}

void ConnectionPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(30, 30, 30, 40));
    
    // Рамка панели
    painter.setPen(QPen(QColor(255, 200, 100, 200), 2));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
