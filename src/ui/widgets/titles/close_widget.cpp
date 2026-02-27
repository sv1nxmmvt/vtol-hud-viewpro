#include "close_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

CloseWidget::CloseWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(30, 30);
}

void CloseWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    event->accept();
}

void CloseWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Полупрозрачный фон при наведении
    if (m_hovered) {
        painter.fillRect(rect(), QColor(232, 17, 35, 200));
    } else {
        painter.fillRect(rect(), QColor(232, 17, 35, 100));
    }
    
    // Рисуем крестик
    painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    int margin = 12;
    painter.drawLine(margin, margin, width() - margin, height() - margin);
    painter.drawLine(width() - margin, margin, margin, height() - margin);
}

void CloseWidget::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void CloseWidget::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
