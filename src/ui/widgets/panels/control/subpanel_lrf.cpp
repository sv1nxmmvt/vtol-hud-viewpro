#include "subpanel_lrf.h"
#include <QPainter>

LrfSubPanel::LrfSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void LrfSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Только рамка, без фона
    painter.setPen(QPen(QColor(255, 100, 100, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
