#pragma once

#include <QObject>
#include <QMutex>
#include <memory>

#include "gimbal.h"

namespace gimbal {

/**
 * @brief Поток получения телеметрии от подвеса
 * @details Получает телеметрию от подвеса через callback и предоставляет
 *          доступ к текущим значениям для UI и других компонентов.
 *
 * @note Класс использует singleton для доступа к данным телеметрии
 */
class TelemetryStream : public QObject {
    Q_OBJECT

public:
    explicit TelemetryStream(QObject* parent = nullptr);
    ~TelemetryStream() override;

    /**
     * @brief Инициализировать TelemetryStream
     * @param gimbal Указатель на Gimbal для получения телеметрии
     */
    void init(std::shared_ptr<Gimbal> gimbal);

    /**
     * @brief Проверить, инициализирован ли TelemetryStream
     */
    bool isInitialized() const;

    /**
     * @brief Получить экземпляр TelemetryStream
     */
    static TelemetryStream& instance();

    /**
     * @brief Получить текущую телеметрию
     * @return Текущие данные телеметрии
     */
    Telemetry getTelemetry() const;

    /**
     * @brief Получить текущие углы подвеса
     * @param yaw Указатель на переменную для yaw (может быть nullptr)
     * @param pitch Указатель на переменную для pitch (может быть nullptr)
     * @param roll Указатель на переменную для roll (может быть nullptr)
     */
    void getAngles(double* yaw, double* pitch, double* roll) const;

    /**
     * @brief Получить текущий зум
     * @return Кратность зума
     */
    double getZoom() const;

    /**
     * @brief Получить текущий тип сенсора
     * @return Тип сенсора
     */
    SensorType getSensorType() const;

    /**
     * @brief Получить статус трекера
     * @return Статус трекера
     */
    TrackerStatus getTrackerStatus() const;

    /**
     * @brief Получить координаты цели
     * @param lat Указатель на переменную для широты (может быть nullptr)
     * @param lng Указатель на переменную для долготы (может быть nullptr)
     * @param alt Указатель на переменную для высоты (может быть nullptr)
     */
    void getTargetCoordinates(double* lat, double* lng, double* alt) const;

    /**
     * @brief Проверить, активен ли трекинг
     * @return true если трекинг активен
     */
    bool isTracking() const;

signals:
    /**
     * @brief Сигнал обновления телеметрии
     * @param telemetry Новые данные телеметрии
     */
    void telemetryUpdated(const Telemetry& telemetry);

private slots:
    /**
     * @brief Обработка телеметрии от подвеса
     * @param telemetry Данные телеметрии
     */
    void onTelemetryReceived(const Telemetry& telemetry);

private:
    std::weak_ptr<Gimbal> m_gimbal;
    Telemetry m_currentTelemetry;
    mutable QMutex m_mutex;
    bool m_initialized = false;

    static TelemetryStream* s_instance;
};

} // namespace gimbal
