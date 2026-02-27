#include "video_widget.h"
#include "../transparent/transparent_widget.h"
#include <QPainter>
#include <QResizeEvent>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
{
    // Фон по умолчанию (чёрный)
    setStyleSheet("background-color: black;");
    
    // Принимаем события мыши
    setMouseTracking(true);

    // Создаём прозрачный виджет поверх видео
    m_transparentWidget = new TransparentWidget(this);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
}

void VideoWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
    emit resized();
}
