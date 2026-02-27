#include "toggle_button.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

ToggleButton::ToggleButton(QWidget *parent, const QString& text)
    : QWidget(parent)
    , m_text(text)
{
    setFixedSize(80, 33);
    setMouseTracking(true);
}

void ToggleButton::setState(bool state)
{
    m_state = state;
    update();
}

void ToggleButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Цвет фона
    QColor bgColor;
    if (m_state) {
        bgColor = QColor(34, 139, 34, 200);  // Зелёный когда включено
    } else if (m_hovered) {
        bgColor = QColor(100, 150, 200, 150);
    } else {
        bgColor = QColor(60, 60, 60, 120);
    }
    
    // Закруглённый прямоугольник
    QPainterPath path;
    int radius = 10;
    path.addRoundedRect(rect(), radius, radius);
    painter.fillPath(path, bgColor);
    
    // Рамка
    painter.setPen(QPen(m_state ? QColor(100, 255, 100) : QColor(150, 150, 150), 2));
    painter.drawPath(path);
    
    // Текст
    painter.setPen(m_state ? QColor(200, 255, 200) : Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(8);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

void ToggleButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_state = !m_state;
        emit toggled(m_state);
        qDebug() << "[ToggleButton] Toggled to:" << (m_state ? "ON" : "OFF");
        update();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}
