#pragma once

#include <QWidget>
#include <QString>

#include "gimbal/mavlink_stream.h"

/**
 * @class TelemetryPanel
 * @brief Виджет для отображения телеметрии ArduPilot
 *
 * Полупрозрачная панель с основными параметрами телеметрии:
 * - Координаты GPS (широта, долгота)
 * - Высота (относительная)
 * - Скорость
 * - Ориентация (roll, pitch, yaw)
 * - Батарея (процент, напряжение, ток)
 * - Режим полета
 * - Статус вооружения
 * - GPS статус (фикс, спутники, HDOP)
 * - RC статус (доступность, RSSI)
 */
class TelemetryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TelemetryPanel(QWidget *parent = nullptr);
    ~TelemetryPanel() override = default;

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
     * @brief Отрисовать все параметры телеметрии
     */
    void drawTelemetryData(QPainter& painter);

    /**
     * @brief Отрисовать одну строку с параметром
     * @param painter QPainter для отрисовки
     * @param label Название параметра
     * @param value Значение параметра
     * @param y Позиция по вертикали
     */
    void drawParameterRow(QPainter& painter, const QString& label, const QString& value, int y);

    /**
     * @brief Форматировать координаты GPS
     */
    QString formatCoordinates(double lat, double lon) const;

    /**
     * @brief Форматировать высоту
     */
    QString formatAltitude(double altitude) const;

    /**
     * @brief Форматировать скорость
     */
    QString formatSpeed(double speed) const;

    /**
     * @brief Форматировать углы ориентации
     */
    QString formatAngle(double angle) const;

    /**
     * @brief Форматировать данные батареи
     */
    QString formatBattery(float percent, float voltage, float current) const;

    /**
     * @brief Форматировать статус GPS
     */
    QString formatGpsStatus(int fixType, int satellites, float hdop) const;

    /**
     * @brief Форматировать статус RC
     */
    QString formatRcStatus(bool available, float rssi) const;

    gimbal::MavlinkTelemetry m_telemetry;
    bool m_visible = false;

    // Цвета
    static constexpr int BG_ALPHA = 10;  // Почти прозрачный фон
    static const QColor TEXT_COLOR;        // Салатовый текст
    static const QColor BG_COLOR;          // Цвет фона (под цвет кнопки)
};
