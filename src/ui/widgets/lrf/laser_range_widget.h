#pragma once

#include <QWidget>
#include <QLabel>

/**
 * @brief Виджет отображения расстояния от лазерного дальномера
 * @details Отображает расстояние в метрах с полупрозрачным фоном
 */
class LaserRangeWidget : public QWidget {
    Q_OBJECT

public:
    explicit LaserRangeWidget(QWidget* parent = nullptr);
    ~LaserRangeWidget() override;

    /**
     * @brief Установить расстояние
     * @param distance Расстояние в метрах
     */
    void setDistance(double distance);

    /**
     * @brief Получить текущее расстояние
     * @return Расстояние в метрах
     */
    double getDistance() const;

    /**
     * @brief Показать виджет с анимацией
     */
    void showWithAnimation();

    /**
     * @brief Скрыть виджет с анимацией
     */
    void hideWithAnimation();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updatePosition();
    void updateStyle();

    QLabel* m_distanceLabel = nullptr;
    double m_distance = 0.0;
    bool m_visible = false;
};
