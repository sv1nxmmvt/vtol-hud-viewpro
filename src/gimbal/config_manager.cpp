#include "config_manager.h"

#include <fstream>
#include <sstream>
#include <cstring>

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

namespace gimbal {

ConnectionConfig ConfigManager::getDefaultConfig() {
    ConnectionConfig config;
    config.type = ConnectionType::Tcp;
    config.ip = "192.168.2.119";
    config.port = 2000;
    config.videoPort = 554;
    config.serialPort = "";
    config.baudRate = 115200;
    return config;
}

std::optional<ConnectionConfig> ConfigManager::load(const std::string& path) {
    // Если путь абсолютный - используем его
    std::string configPath = path;
    
    // Если путь относительный - ищем конфиг рядом с исполняемым файлом
    if (path.empty() || path == DEFAULT_CONFIG_PATH) {
        QString appDir = QCoreApplication::applicationDirPath();
        configPath = QDir(appDir).filePath(DEFAULT_CONFIG_PATH).toStdString();
    }
    
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // Файл не найден, возвращаем конфигурацию по умолчанию
        return getDefaultConfig();
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();

    return fromJson(json);
}

bool ConfigManager::save(const ConnectionConfig& config, const std::string& path) {
    // Если путь относительный - сохраняем рядом с исполняемым файлом
    std::string configPath = path;
    if (path.empty() || path == DEFAULT_CONFIG_PATH) {
        QString appDir = QCoreApplication::applicationDirPath();
        configPath = QDir(appDir).filePath(DEFAULT_CONFIG_PATH).toStdString();
    }
    
    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    std::string json = toJson(config);
    file << json;
    file.close();

    return true;
}

std::string ConfigManager::toJson(const ConnectionConfig& config) const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"type\": ";
    switch (config.type) {
        case ConnectionType::Tcp: oss << "\"tcp\""; break;
        case ConnectionType::Udp: oss << "\"udp\""; break;
        case ConnectionType::SerialPort: oss << "\"serial\""; break;
        default: oss << "\"tcp\""; break;
    }
    oss << ",\n";
    oss << "  \"ip\": \"" << config.ip << "\",\n";
    oss << "  \"port\": " << config.port << ",\n";
    oss << "  \"video_port\": " << config.videoPort << ",\n";
    oss << "  \"serial_port\": \"" << config.serialPort << "\",\n";
    oss << "  \"baud_rate\": " << config.baudRate << "\n";
    oss << "}\n";
    return oss.str();
}

std::optional<ConnectionConfig> ConfigManager::fromJson(const std::string& json) const {
    ConnectionConfig config = getDefaultConfig();

    // Простой парсинг JSON без внешних зависимостей
    auto findValue = [&json](const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) {
            return "";
        }
        pos = json.find(':', pos);
        if (pos == std::string::npos) {
            return "";
        }
        pos++;
        // Пропускаем пробелы
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) {
            pos++;
        }
        if (pos >= json.size()) {
            return "";
        }

        // Определяем тип значения
        if (json[pos] == '"') {
            // Строка
            size_t endPos = json.find('"', pos + 1);
            if (endPos == std::string::npos) {
                return "";
            }
            return json.substr(pos + 1, endPos - pos - 1);
        } else {
            // Число или другое
            size_t endPos = pos;
            while (endPos < json.size() && json[endPos] != ',' && json[endPos] != '\n' && json[endPos] != '}') {
                endPos++;
            }
            std::string value = json.substr(pos, endPos - pos);
            // Trim whitespace
            size_t start = value.find_first_not_of(" \t\n\r");
            size_t end = value.find_last_not_of(" \t\n\r");
            if (start == std::string::npos) {
                return "";
            }
            return value.substr(start, end - start + 1);
        }
    };

    std::string typeStr = findValue("type");
    if (typeStr == "udp") {
        config.type = ConnectionType::Udp;
    } else if (typeStr == "serial") {
        config.type = ConnectionType::SerialPort;
    } else {
        config.type = ConnectionType::Tcp;
    }

    std::string ip = findValue("ip");
    if (!ip.empty()) {
        config.ip = ip;
    }

    std::string portStr = findValue("port");
    if (!portStr.empty()) {
        try {
            config.port = std::stoi(portStr);
        } catch (...) {
            // Оставляем значение по умолчанию
        }
    }

    std::string videoPortStr = findValue("video_port");
    if (!videoPortStr.empty()) {
        try {
            config.videoPort = std::stoi(videoPortStr);
        } catch (...) {
            // Оставляем значение по умолчанию
        }
    }

    std::string serialPort = findValue("serial_port");
    if (!serialPort.empty()) {
        config.serialPort = serialPort;
    }

    std::string baudRateStr = findValue("baud_rate");
    if (!baudRateStr.empty()) {
        try {
            config.baudRate = std::stoi(baudRateStr);
        } catch (...) {
            // Оставляем значение по умолчанию
        }
    }

    return config;
}

} // namespace gimbal
