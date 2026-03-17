#pragma once

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <memory>
#include <optional>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/gimbal/gimbal.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>

namespace gimbal {

/**
 * @brief Структура основной телеметрии от ArduPilot
 */
struct MavlinkTelemetry {
    // Позиция
    double latitude_deg = 0.0;
    double longitude_deg = 0.0;
    double absolute_altitude_m = 0.0;
    double relative_altitude_m = 0.0;
    
    // Ориентация
    double roll_deg = 0.0;
    double pitch_deg = 0.0;
    double yaw_deg = 0.0;
    
    // Скорость
    double velocity_x_m_s = 0.0;
    double velocity_y_m_s = 0.0;
    double velocity_z_m_s = 0.0;
    double speed_m_s = 0.0;
    
    // Батарея
    float battery_percentage = 0.0f;
    float battery_voltage_v = 0.0f;
    float battery_current_a = 0.0f;
    float battery_consumed_ah = 0.0f;
    
    // Режим полета
    std::string flight_mode;
    
    // Статус вооружения
    bool armed = false;
    
    // GPS
    int gps_fix_type = 0;
    int gps_num_satellites = 0;
    float gps_hdop = 0.0f;
    
    // RC
    bool rc_available = false;
    float rc_rssi_percent = 0.0f;

    // Home position
    double home_latitude_deg = 0.0;
    double home_longitude_deg = 0.0;
    double home_altitude_m = 0.0;
    bool home_position_valid = false;

    // Расстояние до дома (2D, в метрах)
    float distance_to_home_m = 0.0f;

    // Вертикальная скорость (climb rate, м/с)
    float vertical_speed_m_s = 0.0f;

    // Статус подключения
    bool connected = false;

    // Сырые MAVLink сообщения (для отладки)
    uint64_t message_count = 0;
};

/**
 * @brief Класс для получения телеметрии ArduPilot через MAVSDK
 * 
 * Подключается по UDP на порту 14552 и получает телеметрию от ArduPilot.
 */
class MavlinkStream : public QObject {
    Q_OBJECT

public:
    explicit MavlinkStream(QObject* parent = nullptr);
    ~MavlinkStream() override;

    /**
     * @brief Подключиться к ArduPilot
     * @param udp_port Порт для прослушивания (по умолчанию 14552)
     * @return true если подключение успешно инициировано
     */
    bool connect(int udp_port = 14552);

    /**
     * @brief Отключиться от ArduPilot
     */
    void disconnect();

    /**
     * @brief Проверить, подключены ли мы
     */
    bool isConnected() const;

    /**
     * @brief Получить текущую телеметрию
     */
    MavlinkTelemetry getTelemetry() const;

    /**
     * @brief Включить подробное логирование MAVLink сообщений
     */
    void setVerboseLogging(bool enabled);

    /**
     * @brief Проверить, вооружен ли аппарат
     */
    bool isArmed() const;

    /**
     * @brief Получить статус GPS
     */
    void getGpsStatus(int* fix_type, int* satellites, float* hdop) const;

    /**
     * @brief Получить статус RC
     */
    void getRcStatus(bool* available, float* rssi_percent) const;

signals:
    /**
     * @brief Сигнал обновления телеметрии
     * @param telemetry Новые данные телеметрии
     */
    void telemetryUpdated(const MavlinkTelemetry& telemetry);

    /**
     * @brief Сигнал изменения статуса подключения
     * @param connected true если подключено
     */
    void connectionChanged(bool connected);

    /**
     * @brief Сигнал изменения статуса вооружения
     * @param armed true если вооружен
     */
    void armedChanged(bool armed);

    /**
     * @brief Сигнал о получении сырого MAVLink сообщения (для отладки)
     * @param msg_name Имя сообщения
     * @param count Общее количество полученных сообщений
     */
    void mavlinkMessageReceived(const QString& msg_name, uint64_t count);

private slots:
    void pollTelemetry();

private:
    std::unique_ptr<mavsdk::Mavsdk> m_mavsdk;
    std::shared_ptr<mavsdk::Telemetry> m_telemetry;
    std::shared_ptr<mavsdk::Gimbal> m_gimbal;
    std::shared_ptr<mavsdk::MavlinkDirect> m_mavlink_direct;
    
    MavlinkTelemetry m_currentTelemetry;
    mutable QMutex m_mutex;
    
    QTimer* m_pollTimer = nullptr;
    bool m_connected = false;
    bool m_verboseLogging = false;
    
    uint64_t m_messageCount = 0;
    bool m_lastArmedStatus = false;  // Для отслеживания изменений статуса вооружения
    
    void setupTelemetryCallbacks();
    void logMavlinkMessage(const std::string& name);
};

} // namespace gimbal
