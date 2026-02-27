#include "control_panel.h"
#include <QPainter>

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void ControlPanel::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    update();
}

void ControlPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(30, 30, 30, 40));
}
