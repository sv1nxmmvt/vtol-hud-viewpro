#include "video_widget.h"
#include "../transparent/transparent_widget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

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
    m_videoLabel->setMinimumSize(1, 1);  // Чтобы label не был нулевым
    layout->addWidget(m_videoLabel);

    // Создаём прозрачный виджет поверх видео
    m_transparentWidget = new TransparentWidget(this);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
    
    qDebug() << "VideoWidget: initialized, label:" << m_videoLabel;
}

VideoWidget::~VideoWidget() = default;

void VideoWidget::displayFrame(const QImage& frame) {
    if (!m_videoLabel) {
        qWarning() << "VideoWidget: [ERROR] m_videoLabel is null";
        return;
    }
    
    if (frame.isNull()) {
        qWarning() << "VideoWidget: [ERROR] frame is null";
        return;
    }
    
    static int displayCount = 0;
    displayCount++;
    
    if (displayCount <= 5 || displayCount % 30 == 0) {
        qDebug() << "VideoWidget: [DISPLAY] frame #" << displayCount
                 << ", input size:" << frame.size()
                 << ", label size:" << m_videoLabel->size()
                 << ", widget size:" << size();
    }

    // Масштабируем кадр по размеру label с сохранением пропорций
    QImage scaledFrame = frame.scaled(
        m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    
    if (displayCount <= 5 || displayCount % 30 == 0) {
        qDebug() << "VideoWidget: [DISPLAY] scaled size:" << scaledFrame.size()
                 << ", isNull:" << scaledFrame.isNull();
    }

    m_videoLabel->setPixmap(QPixmap::fromImage(scaledFrame));
    
    if (displayCount <= 5) {
        qDebug() << "VideoWidget: [DISPLAY] pixmap set on label";
    }
}

void VideoWidget::clearDisplay() {
    qDebug() << "VideoWidget: clearing display";
    if (m_videoLabel) {
        m_videoLabel->clear();
    }
}

QSize VideoWidget::videoSize() const {
    if (m_videoLabel) {
        auto pm = m_videoLabel->pixmap();
        if (!pm.isNull()) {
            return pm.size();
        }
        return m_videoLabel->size();
    }
    return QSize(0, 0);
}

void VideoWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_transparentWidget->setGeometry(rect());
    m_transparentWidget->raise();
    emit resized();
}
