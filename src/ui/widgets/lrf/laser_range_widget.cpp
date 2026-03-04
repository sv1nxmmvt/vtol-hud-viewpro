#include "laser_range_widget.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

LaserRangeWidget::LaserRangeWidget(QWidget* parent)
    : QWidget(parent)
{
    // Настраиваем виджет
    setAttribute(Qt::WA_TransparentForMouseEvents);  // Пропускаем клики
    setAttribute(Qt::WA_TranslucentBackground);      // Прозрачный фон для отрисовки
    
    // Создаём лейбл для отображения расстояния
    m_distanceLabel = new QLabel(this);
    m_distanceLabel->setAlignment(Qt::AlignCenter);
    m_distanceLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    
    // Настраиваем стиль
    updateStyle();
    
    // Создаём эффект прозрачности для анимации
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    setGraphicsEffect(opacityEffect);
    
    // Скрываем изначально
    hide();
}

LaserRangeWidget::~LaserRangeWidget() = default;

void LaserRangeWidget::setDistance(double distance) {
    m_distance = distance;
    m_distanceLabel->setText(QString::number(distance, 'f', 1) + " м");
    
    // Показываем виджет если расстояние > 0
    if (distance > 0.0 && !m_visible) {
        showWithAnimation();
    } else if (distance <= 0.0 && m_visible) {
        hideWithAnimation();
    }
}

double LaserRangeWidget::getDistance() const {
    return m_distance;
}

void LaserRangeWidget::showWithAnimation() {
    m_visible = true;
    show();
    
    auto* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (opacityEffect) {
        auto* animation = new QPropertyAnimation(opacityEffect, "opacity", this);
        animation->setDuration(300);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void LaserRangeWidget::hideWithAnimation() {
    m_visible = false;
    
    auto* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (opacityEffect) {
        auto* animation = new QPropertyAnimation(opacityEffect, "opacity", this);
        animation->setDuration(300);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            hide();
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void LaserRangeWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Полупрозрачный фон
    QColor bgColor(0, 0, 0, 180);  // Чёрный с 70% прозрачностью
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    
    // Рисуем скруглённый прямоугольник
    QRectF rect = this->rect().adjusted(2, 2, -2, -2);
    painter.drawRoundedRect(rect, 8, 8);
    
    // Рамка
    QColor borderColor(255, 255, 255, 100);  // Белая с 40% прозрачностью
    painter.setPen(QPen(borderColor, 1));
    painter.drawRoundedRect(rect, 8, 8);
}

void LaserRangeWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePosition();
}

void LaserRangeWidget::updatePosition() {
    // Позиционирование: центр по горизонтали, 1/4 от верха
    if (parentWidget()) {
        int parentWidth = parentWidget()->width();
        int parentHeight = parentWidget()->height();
        
        // Вычисляем позицию
        int x = (parentWidth - width()) / 2;
        int y = parentHeight / 4 - height() / 2;
        
        move(x, y);
    }
}

void LaserRangeWidget::updateStyle() {
    // Стиль для лейбла
    m_distanceLabel->setStyleSheet(
        "QLabel {"
        "    color: #FFFFFF;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "    background: transparent;"
        "}"
    );
    
    // Фиксированный размер по содержимому + отступы
    m_distanceLabel->adjustSize();
    int width = m_distanceLabel->width() + 32;   // Отступы по бокам
    int height = m_distanceLabel->height() + 16; // Отступы сверху/снизу
    resize(width, height);
    
    // Размещаем лейбл по центру
    m_distanceLabel->move((width - m_distanceLabel->width()) / 2,
                          (height - m_distanceLabel->height()) / 2);
}
