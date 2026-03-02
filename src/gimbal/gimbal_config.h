#pragma once

#include <string>
#include <cstdint>

namespace gimbal {

/**
 * @brief Тип подключения к подвесу
 */
enum class ConnectionType {
    Tcp = 0x01,
    Udp = 0x02,
    SerialPort = 0x00
};

/**
 * @brief Конфигурация подключения к подвесу
 */
struct ConnectionConfig {
    ConnectionType type = ConnectionType::Tcp;
    std::string ip = "192.168.2.119";
    int port = 2000;              // Порт для коммуникации (команды/телеметрия)
    int videoPort = 554;          // Порт для видео (RTSP)
    std::string serialPort;       // Имя последовательного порта (для SerialPort)
    int baudRate = 115200;        // Скорость последовательного порта

    /**
     * @brief Получить RTSP URL для видеопотока
     */
    std::string getRtspUrl() const {
        // Формат: rtsp://<ip>:<videoPort>/
        return "rtsp://" + ip + ":" + std::to_string(videoPort) + "/";
    }

    /**
     * @brief Получить строковое представление типа подключения
     */
    std::string getTypeString() const {
        switch (type) {
            case ConnectionType::Tcp: return "TCP";
            case ConnectionType::Udp: return "UDP";
            case ConnectionType::SerialPort: return "Serial";
            default: return "Unknown";
        }
    }
};

/**
 * @brief Путь к файлу конфигурации по умолчанию
 */
constexpr const char* DEFAULT_CONFIG_PATH = "gimbal_config.json";

} // namespace gimbal
