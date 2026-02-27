#include "connection_button.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ConnectionButton::ConnectionButton(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(50, 50);
}

void ConnectionButton::setActive(bool active)
{
    m_active = active;
    update();
}

void ConnectionButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_active = !m_active;
        emit toggled(m_active);
        update();
    }
    event->accept();
}

void ConnectionButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Создаём путь с закруглёнными углами
    QPainterPath path;
    int radius = 10;
    path.addRoundedRect(rect(), radius, radius);
    
    // Цвет фона - полностью прозрачный в неактивном состоянии
    if (m_active) {
        // Активная кнопка - зелёная подсветка
        painter.fillPath(path, QColor(34, 139, 34, 200));
    } else if (m_hovered) {
        // При наведении - слегка видимый
        painter.fillPath(path, QColor(100, 100, 100, 80));
    }
    // В неактивном состоянии - полностью прозрачный (ничего не рисуем)
    
    // Обводка
    painter.setPen(QPen(m_active ? QColor(100, 255, 100) : (m_hovered ? QColor(200, 200, 200) : QColor(150, 150, 150)), 2));
    painter.drawPath(path);
    
    // Иконка подключения (символ вилки/антенны)
    painter.setPen(QPen(m_active ? QColor(100, 255, 100) : Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    int cx = width() / 2;
    int cy = height() / 2;
    
    // Рисуем символ подключения
    painter.drawLine(cx, cy - 10, cx, cy + 5);
    painter.drawLine(cx - 8, cy - 5, cx, cy + 5);
    painter.drawLine(cx + 8, cy - 5, cx, cy + 5);
    
    // Волны сигнала
    if (m_active) {
        painter.setPen(QPen(QColor(100, 255, 100), 1, Qt::SolidLine));
        painter.drawArc(cx - 15, cy - 20, 30, 20, 0, 180 * 16);
        painter.drawArc(cx - 12, cy - 17, 24, 17, 0, 180 * 16);
    }
}

void ConnectionButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ConnectionButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
