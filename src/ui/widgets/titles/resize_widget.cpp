#include "resize_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

ResizeWidget::ResizeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(30, 30);
}

void ResizeWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    event->accept();
}

void ResizeWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Полупрозрачный фон при наведении
    if (m_hovered) {
        painter.fillRect(rect(), QColor(255, 255, 255, 200));
    } else {
        painter.fillRect(rect(), QColor(255, 255, 255, 0));
    }
    
    // Рисуем квадрат (символ maximize/restore)
    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
    int margin = 10;
    painter.drawRect(margin, margin, width() - margin * 2, height() - margin * 2);
}

void ResizeWidget::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void ResizeWidget::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
