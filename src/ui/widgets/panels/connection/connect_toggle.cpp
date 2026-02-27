#include "connect_toggle.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

ConnectToggle::ConnectToggle(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(90, 28);
    setMouseTracking(true);
}

void ConnectToggle::setState(bool connected)
{
    m_connected = connected;
    update();
}

void ConnectToggle::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Цвет фона - КРАСНЫЙ когда подключено (DISCONNECT), зелёный когда отключено (CONNECT)
    QColor bgColor;
    if (m_connected) {
        bgColor = QColor(180, 50, 50, 200);  // Красный когда подключено
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
    painter.setPen(QPen(m_connected ? QColor(255, 100, 100) : QColor(150, 150, 150), 1));
    painter.drawPath(path);
    
    // Текст
    QString text = m_connected ? "DISCONNECT" : "CONNECT";
    painter.setPen(m_connected ? QColor(255, 200, 200) : Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(7);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, text);
}

void ConnectToggle::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_connected = !m_connected;
        emit toggled(m_connected);
        qDebug() << "[ConnectToggle] Toggled to:" << (m_connected ? "CONNECTED" : "DISCONNECTED");
        update();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}
