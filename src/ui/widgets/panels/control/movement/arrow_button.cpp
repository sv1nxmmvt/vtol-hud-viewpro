#include "arrow_button.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

ArrowButton::ArrowButton(QWidget *parent, const QString& direction)
    : QWidget(parent)
    , m_direction(direction)
{
    setFixedSize(35, 33);
    setMouseTracking(true);
}

void ArrowButton::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    update();
}

void ArrowButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Цвет фона
    QColor bgColor;
    if (m_pressed) {
        bgColor = QColor(150, 200, 255, 200);
    } else if (m_hovered) {
        bgColor = QColor(100, 150, 200, 150);
    } else if (m_highlighted) {
        bgColor = QColor(80, 120, 160, 150);
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
    
    // Стрелка
    painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    int cx = width() / 2;
    int cy = height() / 2;
    int arrowSize = 10;
    
    if (m_direction == "up") {
        painter.drawLine(cx, cy - arrowSize/2, cx - arrowSize/2, cy + arrowSize/2);
        painter.drawLine(cx, cy - arrowSize/2, cx + arrowSize/2, cy + arrowSize/2);
    } else if (m_direction == "down") {
        painter.drawLine(cx, cy + arrowSize/2, cx - arrowSize/2, cy - arrowSize/2);
        painter.drawLine(cx, cy + arrowSize/2, cx + arrowSize/2, cy - arrowSize/2);
    } else if (m_direction == "left") {
        painter.drawLine(cx - arrowSize/2, cy, cx + arrowSize/2, cy - arrowSize/2);
        painter.drawLine(cx - arrowSize/2, cy, cx + arrowSize/2, cy + arrowSize/2);
    } else if (m_direction == "right") {
        painter.drawLine(cx + arrowSize/2, cy, cx - arrowSize/2, cy - arrowSize/2);
        painter.drawLine(cx + arrowSize/2, cy, cx - arrowSize/2, cy + arrowSize/2);
    }
}

void ArrowButton::mousePressEvent(QMouseEvent* event)
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

void ArrowButton::mouseReleaseEvent(QMouseEvent* event)
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

void ArrowButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ArrowButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
}
