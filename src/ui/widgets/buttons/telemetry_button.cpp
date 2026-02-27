#include "telemetry_button.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

TelemetryButton::TelemetryButton(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(50, 50);
}

void TelemetryButton::setActive(bool active)
{
    m_active = active;
    update();
}

void TelemetryButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_active = !m_active;
        emit toggled(m_active);
        update();
    }
    event->accept();
}

void TelemetryButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Создаём путь с закруглёнными углами
    QPainterPath path;
    int radius = 10;
    path.addRoundedRect(rect(), radius, radius);
    
    // Цвет фона - полностью прозрачный в неактивном состоянии
    if (m_active) {
        // Активная кнопка - синяя подсветка
        painter.fillPath(path, QColor(30, 144, 255, 200));
    } else if (m_hovered) {
        // При наведении - слегка видимый
        painter.fillPath(path, QColor(100, 100, 100, 80));
    }
    // В неактивном состоянии - полностью прозрачный (ничего не рисуем)
    
    // Обводка
    painter.setPen(QPen(m_active ? QColor(150, 200, 255) : (m_hovered ? QColor(200, 200, 200) : QColor(150, 150, 150)), 2));
    painter.drawPath(path);
    
    // Иконка телеметрии (график/линии данных)
    painter.setPen(QPen(m_active ? QColor(150, 200, 255) : Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    int cx = width() / 2;
    int cy = height() / 2;
    
    // Рисуем символ графика
    painter.drawLine(cx - 15, cy + 10, cx - 15, cy - 5);
    painter.drawLine(cx - 15, cy - 5, cx - 5, cy + 5);
    painter.drawLine(cx - 5, cy + 5, cx + 5, cy - 8);
    painter.drawLine(cx + 5, cy - 8, cx + 15, cy + 2);
    painter.drawLine(cx + 15, cy + 2, cx + 15, cy + 10);
    
    // Точки данных
    if (m_active) {
        painter.setBrush(QColor(150, 200, 255));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(cx - 15, cy - 5, 4, 4);
        painter.drawEllipse(cx - 5, cy + 5, 4, 4);
        painter.drawEllipse(cx + 5, cy - 8, 4, 4);
        painter.drawEllipse(cx + 15, cy + 2, 4, 4);
    }
}

void TelemetryButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void TelemetryButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
