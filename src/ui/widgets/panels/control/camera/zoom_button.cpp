#include "zoom_button.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

ZoomButton::ZoomButton(QWidget *parent, const QString& text)
    : QWidget(parent)
    , m_text(text)
{
    setFixedSize(50, 28);
    setMouseTracking(true);
    // Убираем WA_TransparentForMouseEvents, чтобы кнопка могла получать события
}

void ZoomButton::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    update();
}

void ZoomButton::paintEvent(QPaintEvent* event)
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
    
    painter.fillRect(rect(), bgColor);
    
    // Рамка
    painter.setPen(QPen(m_pressed ? QColor(200, 255, 255) : QColor(150, 150, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Текст
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

void ZoomButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        emit pressed();
        update();
        event->accept();  // Блокируем распространение события
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ZoomButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_pressed) {
        m_pressed = false;
        emit released();
        update();
        event->accept();  // Блокируем распространение события
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void ZoomButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ZoomButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
}
