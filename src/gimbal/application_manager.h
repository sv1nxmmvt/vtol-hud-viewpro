#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

#include "gimbal.h"
#include "video_stream.h"
#include "config_manager.h"
#include "control_stream.h"
#include "keyboard_handler.h"
#include "joystick_handler.h"
#include "command_handler.h"
#include "mavlink_stream.h"

namespace gimbal {

/**
 * @brief Менеджер приложения - управляет всеми сервисами и обработчиками
 * 
 * Этот класс инкапсулирует инициализацию и управление всеми компонентами подвеса,
 * отделяя бизнес-логику от UI.
 */
class ApplicationManager : public QObject {
    Q_OBJECT

public:
    explicit ApplicationManager(QObject* parent = nullptr);
    ~ApplicationManager() override;

    /**
     * @brief Инициализировать все компоненты
     * @return true если успешо
     */
    bool initialize();

    /**
     * @brief Остановить и очистить все компоненты
     */
    void shutdown();

    /**
     * @brief Получить указатель на Gimbal
     */
    std::shared_ptr<Gimbal> gimbal() const { return m_gimbal; }

    /**
     * @brief Получить указатель на VideoStream
     */
    VideoStream* videoStream() const { return m_videoStream.get(); }

    /**
     * @brief Получить указатель на ConfigManager
     */
    ConfigManager* configManager() const { return m_configManager.get(); }

    /**
     * @brief Получить указатель на KeyboardHandler
     */
    KeyboardHandler* keyboardHandler() const { return m_keyboardHandler.get(); }

    /**
     * @brief Получить указатель на JoystickHandler
     */
    JoystickHandler* joystickHandler() const { return m_joystickHandler.get(); }

    /**
     * @brief Получить указатель на ControlStream
     */
    ControlStream* controlStream() const { return m_controlStream; }

    /**
     * @brief Получить указатель на MavlinkStream
     */
    MavlinkStream* mavlinkStream() const { return m_mavlinkStream.get(); }

    /**
     * @brief Получить конфигурацию подключения
     */
    ConnectionConfig config() const { return m_config; }

    /**
     * @brief Загрузить конфигурацию
     * @return true если успешно загружена
     */
    bool loadConfig();

    /**
     * @brief Подключиться к подвесу
     * @return true если подключение успешно инициировано
     */
    bool connectToGimbal();

    /**
     * @brief Отключиться от подвеса
     */
    void disconnectFromGimbal();

    /**
     * @brief Проверить, подключен ли подвес
     */
    bool isConnected() const { return m_isConnected; }

    /**
     * @brief Проверить, подключен ли ArduPilot
     */
    bool isArduPilotConnected() const;

signals:
    /**
     * @brief Сигнал изменения статуса подключения к подвесу
     */
    void gimbalConnectionChanged(bool connected);

    /**
     * @brief Сигнал изменения статуса подключения к ArduPilot
     */
    void arduPilotConnectionChanged(bool connected);

    /**
     * @brief Сигнал статуса подключения (от ViewLink SDK)
     */
    void connectionStatusChanged(ConnectionStatus status);

private:
    // Компоненты
    std::shared_ptr<Gimbal> m_gimbal;
    std::unique_ptr<VideoStream> m_videoStream;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<KeyboardHandler> m_keyboardHandler;
    std::unique_ptr<JoystickHandler> m_joystickHandler;
    std::unique_ptr<MavlinkStream> m_mavlinkStream;
    ControlStream* m_controlStream = nullptr;  // Singleton, не владеем
    
    ConnectionConfig m_config;
    bool m_isConnected = false;
    bool m_isInitialized = false;
    
    QTimer* m_connectionTimeoutTimer = nullptr;
    bool m_connectionPending = false;

    // Внутренние методы
    void initGimbalComponents();
    void cleanupGimbalComponents();
    void startControlStream();
    void stopControlStream();
    
    // Обработчики
    void onConnectionStatusChanged(ConnectionStatus status);
    void onGimbalTelemetryReceived(const Telemetry& telemetry);
};

} // namespace gimbal
