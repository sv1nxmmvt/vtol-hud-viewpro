#include "action_button.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

ActionButton::ActionButton(QWidget *parent, const QString& text)
    : QWidget(parent)
    , m_text(text)
{
    setFixedSize(80, 33);
    setMouseTracking(true);
}

void ActionButton::setPressed(bool pressed)
{
    m_pressed = pressed;
    update();
}

void ActionButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Цвет фона
    QColor bgColor;
    if (m_pressed) {
        bgColor = QColor(150, 200, 255, 200);
    } else if (m_hovered) {
        bgColor = QColor(100, 150, 200, 150);
    } else {
        bgColor = QColor(60, 60, 60, 120);
    }
    
    // Закруглённый прямоугольник
    QPainterPath path;
    int radius = 8;
    path.addRoundedRect(rect(), radius, radius);
    painter.fillPath(path, bgColor);
    
    // Рамка
    painter.setPen(QPen(m_pressed ? QColor(200, 255, 255) : QColor(150, 150, 150), 1));
    painter.drawPath(path);
    
    // Текст
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

void ActionButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        emit pressed();
        update();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ActionButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_pressed) {
        m_pressed = false;
        emit released();
        update();
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void ActionButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ActionButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
}
