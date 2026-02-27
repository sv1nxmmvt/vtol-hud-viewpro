#include "telemetry_panel.h"
#include <QPainter>

TelemetryPanel::TelemetryPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void TelemetryPanel::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    update();
}

void TelemetryPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(30, 30, 30, 40));
}
