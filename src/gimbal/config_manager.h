#pragma once

#include "gimbal_config.h"
#include <string>
#include <optional>

namespace gimbal {

/**
 * @class ConfigManager
 * @brief Менеджер конфигурации подключения к подвесу
 *
 * Отвечает за загрузку и сохранение конфигурации в JSON файл.
 * Путь к конфигу определяется относительно исполняемого файла или рабочей директории.
 */
class ConfigManager {
public:
    ConfigManager() = default;
    ~ConfigManager() = default;

    /**
     * @brief Загрузить конфигурацию из файла
     * @param path Путь к файлу конфигурации
     * @return Загруженная конфигурация или std::nullopt при ошибке
     */
    std::optional<ConnectionConfig> load(const std::string& path = DEFAULT_CONFIG_PATH);

    /**
     * @brief Сохранить конфигурацию в файл
     * @param config Конфигурация для сохранения
     * @param path Путь к файлу конфигурации
     * @return true при успехе, false при ошибке
     */
    bool save(const ConnectionConfig& config, const std::string& path = DEFAULT_CONFIG_PATH);

    /**
     * @brief Получить конфигурацию по умолчанию
     * @return Конфигурация по умолчанию
     */
    static ConnectionConfig getDefaultConfig();

private:
    /**
     * @brief Преобразовать ConnectionConfig в JSON строку
     */
    std::string toJson(const ConnectionConfig& config) const;

    /**
     * @brief Преобразовать JSON строку в ConnectionConfig
     * @return Конфигурация или std::nullopt при ошибке парсинга
     */
    std::optional<ConnectionConfig> fromJson(const std::string& json) const;
};

} // namespace gimbal
