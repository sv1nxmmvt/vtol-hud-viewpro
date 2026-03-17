#pragma once

#include <QWidget>
#include <QString>

#include "gimbal/mavlink_stream.h"

/**
 * @class DirectionPanel
 * @brief Панель для отображения ориентации аппарата (направление, тангаж, крен)
 *
 * Отображает три индикатора:
 * - Компас (yaw) - направление относительно севера
 * - Тангаж (pitch) - наклон вперёд/назад
 * - Крен (roll) - наклон влево/вправо
 */
class DirectionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DirectionPanel(QWidget *parent = nullptr);
    ~DirectionPanel() override = default;

    /**
     * @brief Обновить данные ориентации
     * @param telemetry Данные телеметрии
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
    gimbal::MavlinkTelemetry m_telemetry;
    bool m_visible = false;

    // Цвета
    static constexpr int BG_ALPHA = 10;
    static const QColor TEXT_COLOR;
    static const QColor BG_COLOR;
    static const QColor INDICATOR_COLOR;

    /**
     * @brief Отрисовать фон панели
     */
    void drawBackground(QPainter& painter);

    /**
     * @brief Отрисовать все индикаторы
     */
    void drawIndicators(QPainter& painter);

    /**
     * @brief Отрисовать индикатор компаса (направление)
     */
    void drawCompassIndicator(QPainter& painter, int x, int y, int size);

    /**
     * @brief Отрисовать индикатор тангажа
     */
    void drawPitchIndicator(QPainter& painter, int x, int y, int size);

    /**
     * @brief Отрисовать индикатор крена
     */
    void drawRollIndicator(QPainter& painter, int x, int y, int size);

    /**
     * @brief Отрисовать подпись индикатора
     */
    void drawIndicatorLabel(QPainter& painter, const QString& label, const QString& value, int x, int y);

    /**
     * @brief Форматировать угол направления
     */
    QString formatDirection(double yaw) const;

    /**
     * @brief Форматировать угол тангажа/крена
     */
    QString formatAngle(double angle) const;
};
