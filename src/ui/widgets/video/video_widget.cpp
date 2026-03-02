#include "video_widget.h"
#include "../transparent/transparent_widget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QLabel>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
{
    // Фон по умолчанию (чёрный)
    setStyleSheet("background-color: black;");

    // Создаём layout для размещения видео
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Создаём QLabel для отображения видео
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background-color: black;");
    layout->addWidget(m_videoLabel);

    // Создаём прозрачный виджет поверх видео
    m_transparentWidget = new TransparentWidget(this);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
}

VideoWidget::~VideoWidget() = default;

void VideoWidget::displayFrame(const QImage& frame) {
    if (!m_videoLabel || frame.isNull()) {
        return;
    }

    // Масштабируем кадр по размеру label с сохранением пропорций
    QImage scaledFrame = frame.scaled(
        m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    m_videoLabel->setPixmap(QPixmap::fromImage(scaledFrame));
}

void VideoWidget::clearDisplay() {
    if (m_videoLabel) {
        m_videoLabel->clear();
    }
}

void VideoWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
    emit resized();
}
