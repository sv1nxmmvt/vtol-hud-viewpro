#include "mavlink_stream.h"

#include <QDebug>
#include <QMutexLocker>
#include <QThread>
#include <sstream>
#include <cmath>

namespace gimbal {

MavlinkStream::MavlinkStream(QObject* parent)
    : QObject(parent)
    , m_lastArmedStatus(false)
{
    qDebug() << "[MavlinkStream] Created, will listen on UDP port 14552";
}

MavlinkStream::~MavlinkStream() {
    disconnect();
}

bool MavlinkStream::connect(int udp_port) {
    if (m_connected) {
        qWarning() << "[MavlinkStream] Already connected";
        return false;
    }

    qDebug() << "[MavlinkStream] Connecting to ArduPilot on UDP port" << udp_port;

    // Создаем экземпляр MAVSDK с конфигурацией
    mavsdk::Mavsdk::Configuration config(mavsdk::ComponentType::GroundStation);
    m_mavsdk = std::make_unique<mavsdk::Mavsdk>(config);
    
    // Подключаемся по UDP (слушаем указанный порт)
    // ArduPilot отправляет телеметрию на этот порт
    std::string connection_url = "udpin://0.0.0.0:" + std::to_string(udp_port);
    
    auto result = m_mavsdk->add_any_connection(connection_url);
    
    if (result != mavsdk::ConnectionResult::Success) {
        std::ostringstream oss;
        oss << result;
        qCritical() << "[MavlinkStream] Connection failed:" 
                    << QString::fromStdString(oss.str());
        return false;
    }

    qDebug() << "[MavlinkStream] Connection initiated, waiting for system...";

    // Ждем появления системы (таймаут 3 секунды)
    QThread::msleep(100);
    
    // Получаем первую доступную систему (автопилот)
    auto systems = m_mavsdk->systems();
    if (systems.empty()) {
        qDebug() << "[MavlinkStream] No systems found yet, waiting...";
        QThread::msleep(2000);
        systems = m_mavsdk->systems();
    }

    if (systems.empty()) {
        qWarning() << "[MavlinkStream] No MAVLink system found. Is ArduPilot sending telemetry?";
        // Не считаем это ошибкой - продолжаем работу, система может появиться позже
    } else {
        auto system = systems[0];
        qDebug() << "[MavlinkStream] Found system with ID:" << static_cast<int>(system->get_system_id());

        // Создаем plugin'ы для работы с системой
        m_telemetry = std::make_shared<mavsdk::Telemetry>(system);
        m_gimbal = std::make_shared<mavsdk::Gimbal>(system);
        m_mavlink_direct = std::make_shared<mavsdk::MavlinkDirect>(system);

        // Настраиваем callback'и
        setupTelemetryCallbacks();

        // Запускаем поллер для периодического опроса телеметрии
        m_pollTimer = new QTimer(this);
        QObject::connect(m_pollTimer, &QTimer::timeout, this, &MavlinkStream::pollTelemetry);
        m_pollTimer->start(100);  // 10 Гц

        m_connected = true;
        emit connectionChanged(true);

        qDebug() << "[MavlinkStream] Connected to ArduPilot!";
    }

    return true;
}

void MavlinkStream::disconnect() {
    if (m_pollTimer) {
        m_pollTimer->stop();
        m_pollTimer->deleteLater();
        m_pollTimer = nullptr;
    }

    m_telemetry.reset();
    m_gimbal.reset();
    m_mavlink_direct.reset();
    m_mavsdk.reset();

    if (m_connected) {
        qDebug() << "[MavlinkStream] Disconnected";
        m_connected = false;
        emit connectionChanged(false);
    }
}

bool MavlinkStream::isConnected() const {
    return m_connected;
}

MavlinkTelemetry MavlinkStream::getTelemetry() const {
    QMutexLocker locker(&m_mutex);
    return m_currentTelemetry;
}

void MavlinkStream::setVerboseLogging(bool enabled) {
    m_verboseLogging = enabled;
    qDebug() << "[MavlinkStream] Verbose logging:" << (enabled ? "ON" : "OFF");
}

bool MavlinkStream::isArmed() const {
    QMutexLocker locker(&m_mutex);
    return m_currentTelemetry.armed;
}

void MavlinkStream::getGpsStatus(int* fix_type, int* satellites, float* hdop) const {
    QMutexLocker locker(&m_mutex);
    if (fix_type) *fix_type = m_currentTelemetry.gps_fix_type;
    if (satellites) *satellites = m_currentTelemetry.gps_num_satellites;
    if (hdop) *hdop = m_currentTelemetry.gps_hdop;
}

void MavlinkStream::getRcStatus(bool* available, float* rssi_percent) const {
    QMutexLocker locker(&m_mutex);
    if (available) *available = m_currentTelemetry.rc_available;
    if (rssi_percent) *rssi_percent = m_currentTelemetry.rc_rssi_percent;
}

void MavlinkStream::pollTelemetry() {
    if (!m_connected || !m_telemetry) {
        return;
    }

    // Получаем телеметрию из MAVSDK plugin'ов
    MavlinkTelemetry telemetry;

    // Позиция (GPS)
    auto position = m_telemetry->position();
    telemetry.latitude_deg = position.latitude_deg;
    telemetry.longitude_deg = position.longitude_deg;
    telemetry.absolute_altitude_m = position.absolute_altitude_m;
    telemetry.relative_altitude_m = position.relative_altitude_m;

    // Ориентация (Euler angles)
    auto attitude = m_telemetry->attitude_euler();
    telemetry.roll_deg = attitude.roll_deg;
    telemetry.pitch_deg = attitude.pitch_deg;
    telemetry.yaw_deg = attitude.yaw_deg;

    // Скорость (NED frame)
    auto velocity_ned = m_telemetry->velocity_ned();
    telemetry.velocity_x_m_s = velocity_ned.north_m_s;
    telemetry.velocity_y_m_s = velocity_ned.east_m_s;
    telemetry.velocity_z_m_s = velocity_ned.down_m_s;
    
    // Скорость по модулю
    telemetry.speed_m_s = std::sqrt(
        velocity_ned.north_m_s * velocity_ned.north_m_s +
        velocity_ned.east_m_s * velocity_ned.east_m_s +
        velocity_ned.down_m_s * velocity_ned.down_m_s
    );

    // Батарея
    auto battery = m_telemetry->battery();
    telemetry.battery_percentage = battery.remaining_percent;
    telemetry.battery_voltage_v = battery.voltage_v;
    telemetry.battery_current_a = battery.current_battery_a;
    telemetry.battery_consumed_ah = battery.capacity_consumed_ah;

    // Режим полета
    auto flight_mode = m_telemetry->flight_mode();
    std::ostringstream oss;
    oss << flight_mode;
    telemetry.flight_mode = oss.str();

    // Статус вооружения
    telemetry.armed = m_telemetry->armed();

    // GPS
    auto gps_info = m_telemetry->gps_info();
    telemetry.gps_fix_type = static_cast<int>(gps_info.fix_type);
    telemetry.gps_num_satellites = gps_info.num_satellites;
    
    // GPS HDOP из raw_gps
    auto raw_gps = m_telemetry->raw_gps();
    telemetry.gps_hdop = raw_gps.hdop;

    // RC
    auto rc_status = m_telemetry->rc_status();
    telemetry.rc_available = rc_status.is_available;
    telemetry.rc_rssi_percent = rc_status.signal_strength_percent;

    // Home position
    auto home_pos = m_telemetry->home();
    telemetry.home_latitude_deg = home_pos.latitude_deg;
    telemetry.home_longitude_deg = home_pos.longitude_deg;
    telemetry.home_altitude_m = home_pos.absolute_altitude_m;
    telemetry.home_position_valid = home_pos.latitude_deg != 0.0 || home_pos.longitude_deg != 0.0;

    // Расстояние до дома (2D, формула гаверсинусов)
    if (telemetry.home_position_valid) {
        constexpr double EARTH_RADIUS_M = 6371000.0;
        double dlat = (telemetry.latitude_deg - telemetry.home_latitude_deg) * M_PI / 180.0;
        double dlon = (telemetry.longitude_deg - telemetry.home_longitude_deg) * M_PI / 180.0;
        double lat1 = telemetry.home_latitude_deg * M_PI / 180.0;
        double lat2 = telemetry.latitude_deg * M_PI / 180.0;

        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::sin(dlon / 2) * std::sin(dlon / 2) * std::cos(lat1) * std::cos(lat2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        telemetry.distance_to_home_m = static_cast<float>(EARTH_RADIUS_M * c);
    } else {
        telemetry.distance_to_home_m = 0.0f;
    }

    // Вертикальная скорость (climb rate, м/с) из position_velocity_ned
    // down_m_s положительное при движении вниз, поэтому инвертируем для climb rate
    auto pos_vel = m_telemetry->position_velocity_ned();
    telemetry.vertical_speed_m_s = -pos_vel.velocity.down_m_s;

    telemetry.connected = true;
    telemetry.message_count = m_messageCount;

    // Сохраняем текущую телеметрию
    {
        QMutexLocker locker(&m_mutex);
        m_currentTelemetry = telemetry;
    }

    // Отправляем сигнал
    emit telemetryUpdated(telemetry);

    // Логирование (каждые 50 обновлений = 5 секунд при 10 Гц)
    static int logCounter = 0;
    if (++logCounter % 50 == 0) {
        qDebug() << "[MavlinkStream] Telemetry:"
                 << "pos=[" << telemetry.latitude_deg << "," << telemetry.longitude_deg << "]"
                 << "alt=" << telemetry.relative_altitude_m << "m"
                 << "att=[r:" << telemetry.roll_deg << " p:" << telemetry.pitch_deg << " y:" << telemetry.yaw_deg << "]"
                 << "speed=" << telemetry.speed_m_s << "m/s"
                 << "bat=" << telemetry.battery_percentage << "%" << telemetry.battery_voltage_v << "V" << telemetry.battery_current_a << "A"
                 << "arming=" << (telemetry.armed ? "ARMED" : "DISARMED")
                 << "gps=" << telemetry.gps_fix_type << "(" << telemetry.gps_num_satellites << "sats, hdop:" << telemetry.gps_hdop << ")"
                 << "rc=" << (telemetry.rc_available ? "OK" : "NO") << telemetry.rc_rssi_percent << "%"
                 << "mode=" << QString::fromStdString(telemetry.flight_mode);
    }
}

void MavlinkStream::setupTelemetryCallbacks() {
    if (!m_telemetry) {
        return;
    }

    // Подписываемся на изменения позиции
    m_telemetry->subscribe_position([this](const mavsdk::Telemetry::Position& pos) {
        m_messageCount++;
        if (m_verboseLogging) {
            logMavlinkMessage("POSITION");
        }
    });

    // Подписываемся на изменения ориентации
    m_telemetry->subscribe_attitude_euler([this](const mavsdk::Telemetry::EulerAngle& att) {
        m_messageCount++;
        if (m_verboseLogging) {
            logMavlinkMessage("ATTITUDE");
        }
    });

    // Подписываемся на изменения скорости (NED frame)
    m_telemetry->subscribe_velocity_ned([this](const mavsdk::Telemetry::VelocityNed& vel) {
        m_messageCount++;
        if (m_verboseLogging) {
            logMavlinkMessage("VELOCITY_NED");
        }
    });

    // Подписываемся на изменения батареи
    m_telemetry->subscribe_battery([this](const mavsdk::Telemetry::Battery& bat) {
        m_messageCount++;
        if (m_verboseLogging) {
            logMavlinkMessage("BATTERY_STATUS");
        }
    });

    // Подписываемся на изменения режима полета
    m_telemetry->subscribe_flight_mode([this](mavsdk::Telemetry::FlightMode mode) {
        m_messageCount++;
        std::ostringstream oss;
        oss << mode;
        qDebug() << "[MavlinkStream] Flight mode changed:" 
                 << QString::fromStdString(oss.str());
        emit mavlinkMessageReceived("FLIGHT_MODE", m_messageCount);
    });

    // Подписываемся на изменения статуса вооружения
    m_telemetry->subscribe_armed([this](bool armed) {
        m_messageCount++;
        // Выводим сообщение об изменении только если статус изменился
        if (armed != m_lastArmedStatus) {
            qDebug() << "[MavlinkStream] Armed status changed:" << (armed ? "ARMED" : "DISARMED");
            emit armedChanged(armed);
            m_lastArmedStatus = armed;
        }
        emit mavlinkMessageReceived("ARMING", m_messageCount);
    });

    // Подписываемся на изменения GPS raw (для отладки)
    m_telemetry->subscribe_gps_info([this](const mavsdk::Telemetry::GpsInfo& gps) {
        m_messageCount++;
        if (m_verboseLogging) {
            qDebug() << "[MavlinkStream] GPS: satellites=" << gps.num_satellites 
                     << "fix=" << static_cast<int>(gps.fix_type);
            logMavlinkMessage("GPS_RAW_INT");
        }
    });

    // Подписываемся на изменения RC (для отладки)
    m_telemetry->subscribe_rc_status([this](const mavsdk::Telemetry::RcStatus& rc) {
        m_messageCount++;
        if (m_verboseLogging) {
            qDebug() << "[MavlinkStream] RC: available=" << rc.is_available
                     << "strength=" << rc.signal_strength_percent << "%";
            logMavlinkMessage("RC_CHANNELS");
        }
    });

    qDebug() << "[MavlinkStream] Telemetry callbacks setup, message count:" << m_messageCount;
}

void MavlinkStream::logMavlinkMessage(const std::string& name) {
    emit mavlinkMessageReceived(QString::fromStdString(name), m_messageCount);
    
    if (m_verboseLogging) {
        qDebug() << "[MavlinkStream] MAVLink msg:" << QString::fromStdString(name)
                 << "(total:" << m_messageCount << ")";
    }
}

} // namespace gimbal
