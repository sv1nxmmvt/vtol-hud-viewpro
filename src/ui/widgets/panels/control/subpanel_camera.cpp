#include "subpanel_camera.h"
#include <QPainter>

CameraSubPanel::CameraSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void CameraSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Только рамка, без фона
    painter.setPen(QPen(QColor(255, 200, 100, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
