#include "direction_button.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

DirectionButton::DirectionButton(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(50, 50);
}

void DirectionButton::setActive(bool active)
{
    m_active = active;
    update();
}

void DirectionButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_active = !m_active;
        emit toggled(m_active);
        update();
    }
    event->accept();
}

void DirectionButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Создаём путь с закруглёнными углами
    QPainterPath path;
    int radius = 10;
    path.addRoundedRect(rect(), radius, radius);

    // Цвет фона - полностью прозрачный в неактивном состоянии
    if (m_active) {
        // Активная кнопка - зелёная подсветка (цвет компаса)
        painter.fillPath(path, QColor(50, 205, 50, 200));
    } else if (m_hovered) {
        // При наведении - слегка видимый
        painter.fillPath(path, QColor(100, 100, 100, 80));
    }
    // В неактивном состоянии - полностью прозрачный (ничего не рисуем)

    // Обводка
    painter.setPen(QPen(m_active ? QColor(150, 255, 150) : (m_hovered ? QColor(200, 200, 200) : QColor(150, 150, 150)), 2));
    painter.drawPath(path);

    // Иконка компаса
    int cx = width() / 2;
    int cy = height() / 2;

    // Рисуем круг компаса
    painter.setPen(QPen(m_active ? QColor(150, 255, 150) : Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx - 18, cy - 18, 36, 36);

    // Точка в центре (север)
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_active ? QColor(150, 255, 150) : Qt::white);
    painter.drawEllipse(cx - 4, cy - 4, 8, 8);
}

void DirectionButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void DirectionButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
