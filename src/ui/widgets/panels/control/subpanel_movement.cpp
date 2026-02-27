#include "subpanel_movement.h"
#include <QPainter>

MovementSubPanel::MovementSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void MovementSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Только рамка, без фона
    painter.setPen(QPen(QColor(100, 200, 255, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
