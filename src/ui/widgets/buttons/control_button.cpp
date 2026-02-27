#include "control_button.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ControlButton::ControlButton(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(50, 50);
}

void ControlButton::setActive(bool active)
{
    m_active = active;
    update();
}

void ControlButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_active = !m_active;
        emit toggled(m_active);
        update();
    }
    event->accept();
}

void ControlButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Создаём путь с закруглёнными углами
    QPainterPath path;
    int radius = 10;
    path.addRoundedRect(rect(), radius, radius);
    
    // Цвет фона - полностью прозрачный в неактивном состоянии
    if (m_active) {
        // Активная кнопка - оранжевая подсветка
        painter.fillPath(path, QColor(255, 140, 0, 200));
    } else if (m_hovered) {
        // При наведении - слегка видимый
        painter.fillPath(path, QColor(100, 100, 100, 80));
    }
    // В неактивном состоянии - полностью прозрачный (ничего не рисуем)
    
    // Обводка
    painter.setPen(QPen(m_active ? QColor(255, 200, 100) : (m_hovered ? QColor(200, 200, 200) : QColor(150, 150, 150)), 2));
    painter.drawPath(path);
    
    // Иконка управления (джойстик/крестовина)
    painter.setPen(QPen(m_active ? QColor(255, 200, 100) : Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    int cx = width() / 2;
    int cy = height() / 2;
    
    // Рисуем крестовину
    painter.drawLine(cx, cy - 15, cx, cy + 15);
    painter.drawLine(cx - 15, cy, cx + 15, cy);
    
    // Стрелки
    painter.drawLine(cx, cy - 15, cx - 5, cy - 10);
    painter.drawLine(cx, cy - 15, cx + 5, cy - 10);
    painter.drawLine(cx, cy + 15, cx - 5, cy + 10);
    painter.drawLine(cx, cy + 15, cx + 5, cy + 10);
    painter.drawLine(cx - 15, cy, cx - 10, cy - 5);
    painter.drawLine(cx - 15, cy, cx - 10, cy + 5);
    painter.drawLine(cx + 15, cy, cx + 10, cy - 5);
    painter.drawLine(cx + 15, cy, cx + 10, cy + 5);
    
    // Центр
    if (m_active) {
        painter.setBrush(QColor(255, 200, 100));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(cx - 5, cy - 5, 10, 10);
    }
}

void ControlButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ControlButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
