#include "hide_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

HideWidget::HideWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(30, 30);
}

void HideWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    event->accept();
}

void HideWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Полупрозрачный фон при наведении
    if (m_hovered) {
        painter.fillRect(rect(), QColor(255, 255, 255, 200));
    } else {
        painter.fillRect(rect(), QColor(255, 255, 255, 0));
    }
    
    // Рисуем линию (символ minimize)
    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
    int y = height() / 2;
    painter.drawLine(12, y, width() - 12, y);
}

void HideWidget::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
}

void HideWidget::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
}
