#pragma once

#include <QWidget>
#include <QString>

#include "gimbal/mavlink_stream.h"

/**
 * @class FlightInfoWidget
 * @brief Виджет для отображения полётной информации
 *
 * Полупрозрачная панель с информацией о полёте:
 * - Высота абсолютная (по барометру)
 * - Высота относительная (от точки взлёта)
 * - Вертикальная скорость
 * - Расстояние до дома (2D)
 */
class FlightInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightInfoWidget(QWidget *parent = nullptr);
    ~FlightInfoWidget() override = default;

    /**
     * @brief Обновить данные телеметрии
     * @param telemetry Новые данные телеметрии
     */
    void updateTelemetry(const gimbal::MavlinkTelemetry& telemetry);

    /**
     * @brief Показать панель
     */
    void showPanel();

    /**
     * @brief Скрыть панель
     */
    void hidePanel();

    /**
     * @brief Проверить, видима ли панель
     */
    bool isPanelVisible() const { return m_visible; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief Отрисовать фон панели
     */
    void drawBackground(QPainter& painter);

    /**
     * @brief Отрисовать все параметры
     */
    void drawFlightData(QPainter& painter);

    /**
     * @brief Отрисовать одну строку с параметром
     */
    void drawParameterRow(QPainter& painter, const QString& label, const QString& value, int y);

    /**
     * @brief Форматировать высоту
     */
    QString formatAltitude(int altitude) const;

    /**
     * @brief Форматировать вертикальную скорость
     */
    QString formatVerticalSpeed(int speed) const;

    /**
     * @brief Форматировать расстояние
     */
    QString formatDistance(int distance) const;

    gimbal::MavlinkTelemetry m_telemetry;
    bool m_visible = true;  // Виджет всегда виден по умолчанию

    // Цвета
    static constexpr int BG_ALPHA = 10;  // Почти прозрачный фон
    static const QColor TEXT_COLOR;        // Салатовый текст
    static const QColor BG_COLOR;          // Цвет фона
};
