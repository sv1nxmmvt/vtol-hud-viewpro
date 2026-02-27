#include "toggle_switch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

ToggleSwitch::ToggleSwitch(QWidget *parent, const QString& textLeft, const QString& textRight)
    : QWidget(parent)
    , m_textLeft(textLeft)
    , m_textRight(textRight)
{
    setFixedSize(80, 28);
    setMouseTracking(true);
    // Убираем WA_TransparentForMouseEvents, чтобы тумблер мог получать события
}

void ToggleSwitch::setState(bool state)
{
    m_state = state;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Фон
    painter.fillRect(rect(), QColor(60, 60, 60, 150));
    
    // Разделительная линия
    painter.setPen(QPen(QColor(150, 150, 150), 1));
    painter.drawLine(width() / 2, 2, width() / 2, height() - 2);
    
    // Рамка
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Текст
    painter.setPen(m_state ? QColor(150, 150, 150) : Qt::white);
    painter.drawText(QRect(0, 0, width() / 2 - 1, height()), Qt::AlignCenter, m_textLeft);
    
    painter.setPen(m_state ? Qt::white : QColor(150, 150, 150));
    painter.drawText(QRect(width() / 2, 0, width() / 2 - 1, height()), Qt::AlignCenter, m_textRight);
    
    // Индикатор
    QRect indicatorRect = m_state 
        ? QRect(width() / 2 + 1, 2, width() / 2 - 3, height() - 4)
        : QRect(2, 2, width() / 2 - 3, height() - 4);
    painter.fillRect(indicatorRect, QColor(100, 200, 100, 150));
}

void ToggleSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_state = !m_state;
        emit toggled(m_state);
        qDebug() << "[ToggleSwitch] Toggled to:" << (m_state ? m_textRight : m_textLeft);
        update();
        event->accept();  // Блокируем распространение события
    } else {
        QWidget::mousePressEvent(event);
    }
}
