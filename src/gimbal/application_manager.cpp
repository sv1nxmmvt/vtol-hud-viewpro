#include "application_manager.h"

#include <QDebug>
#include <QThread>
#include <QTimer>

namespace gimbal {

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "[ApplicationManager] Created";
}

ApplicationManager::~ApplicationManager() {
    shutdown();
}

bool ApplicationManager::initialize() {
    if (m_isInitialized) {
        qWarning() << "[ApplicationManager] Already initialized";
        return false;
    }

    qDebug() << "[ApplicationManager] Initializing...";

    // Инициализируем SDK
    if (!Gimbal::init()) {
        qCritical() << "[ApplicationManager] Failed to initialize Gimbal SDK";
        return false;
    }
    qDebug() << "[ApplicationManager] Gimbal SDK initialized, version:" << Gimbal::getSdkVersion();

    // Создаем таймер таймаута подключения
    m_connectionTimeoutTimer = new QTimer(this);
    m_connectionTimeoutTimer->setSingleShot(true);
    connect(m_connectionTimeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_connectionPending) {
            qWarning() << "[ApplicationManager] Connection timeout - gimbal not responding";
            m_connectionPending = false;
        }
    });

    // Инициализируем компоненты
    initGimbalComponents();

    m_isInitialized = true;
    qDebug() << "[ApplicationManager] Initialized successfully";
    return true;
}

void ApplicationManager::shutdown() {
    if (!m_isInitialized) {
        return;
    }

    qDebug() << "[ApplicationManager] Shutting down...";

    // Отключаемся от подвеса
    disconnectFromGimbal();

    // Очищаем компоненты
    cleanupGimbalComponents();

    // Shutdown SDK
    Gimbal::shutdown();

    m_isInitialized = false;
    qDebug() << "[ApplicationManager] Shutdown complete";
}

bool ApplicationManager::loadConfig() {
    auto configOpt = m_configManager->load();
    if (configOpt) {
        m_config = *configOpt;
        qDebug() << "[ApplicationManager] Configuration loaded:"
                 << QString::fromStdString(m_config.getTypeString())
                 << ", IP:" << QString::fromStdString(m_config.ip)
                 << ", port:" << m_config.port
                 << ", video port:" << m_config.videoPort;
        return true;
    } else {
        m_config = ConfigManager::getDefaultConfig();
        qDebug() << "[ApplicationManager] Using default configuration";
        return false;
    }
}

bool ApplicationManager::connectToGimbal() {
    if (m_connectionPending) {
        qDebug() << "[ApplicationManager] Connection already in progress";
        return false;
    }

    if (!m_gimbal) {
        qCritical() << "[ApplicationManager] Gimbal not initialized";
        return false;
    }

    qDebug() << "[ApplicationManager] === CONNECTING TO GIMBAL ===";
    qDebug() << "[ApplicationManager] [CONFIG] target:" << QString::fromStdString(m_config.ip)
             << ":" << m_config.port
             << ", type:" << QString::fromStdString(m_config.getTypeString())
             << ", videoPort:" << m_config.videoPort;

    // Устанавливаем таймаут подключения (3 секунды)
    m_connectionPending = true;
    m_connectionTimeoutTimer->setInterval(3000);
    m_connectionTimeoutTimer->start();

    // Подключаемся к подвесу
    bool connectResult = m_gimbal->connect(m_config);
    qDebug() << "[ApplicationManager] [GIMBAL] connect() returned:" << (connectResult ? "true" : "false");

    if (!connectResult) {
        qCritical() << "[ApplicationManager] [ERROR] gimbal connect() failed";
        m_connectionPending = false;
        m_connectionTimeoutTimer->stop();
        return false;
    }

    qDebug() << "[ApplicationManager] === CONNECTION INITIATED (waiting for status) ===";
    return true;
}

void ApplicationManager::disconnectFromGimbal() {
    if (!m_isConnected && !m_connectionPending) {
        return;
    }

    qDebug() << "[ApplicationManager] Disconnecting from gimbal";

    // Сначала останавливаем ControlStream
    stopControlStream();

    // Останавливаем видеопоток
    if (m_videoStream) {
        m_videoStream->stop();
    }

    // Останавливаем подвес и отключаемся
    if (m_gimbal) {
        m_gimbal->stop();
        m_gimbal->motorOn(false);
        m_gimbal->disconnect();
    }

    m_isConnected = false;
    m_connectionPending = false;
    emit gimbalConnectionChanged(false);
    qDebug() << "[ApplicationManager] Disconnected from gimbal";
}

bool ApplicationManager::isArduPilotConnected() const {
    return m_mavlinkStream && m_mavlinkStream->isConnected();
}

void ApplicationManager::initGimbalComponents() {
    // Создаём компоненты
    m_gimbal = std::make_shared<Gimbal>();
    m_videoStream = std::make_unique<VideoStream>();
    m_configManager = std::make_unique<ConfigManager>();

    // Инициализируем ControlStream (singleton)
    m_controlStream = &ControlStream::instance(*m_gimbal);
    m_controlStream->setFrequency(10);  // 10 Гц для отзывчивого управления

    // Инициализируем обработчик клавиатуры
    m_keyboardHandler = std::make_unique<KeyboardHandler>(nullptr);

    // Инициализируем обработчик джойстика и запускаем опрос
    m_joystickHandler = std::make_unique<JoystickHandler>(nullptr);
    m_joystickHandler->start();

    // Инициализируем CommandHandler для отправки команд
    CommandHandler::init(m_gimbal);

    // Инициализируем MavlinkStream для получения телеметрии ArduPilot
    m_mavlinkStream = std::make_unique<MavlinkStream>(nullptr);

    // Загружаем конфигурацию
    loadConfig();

    // Настраиваем Keep Alive Interval
    // m_gimbal->setKeepAliveInterval(200);  // Используем дефолтное значение

    // Регистрируем callback для статуса подключения
    m_gimbal->setConnectionCallback([this](ConnectionStatus status) {
        QMetaObject::invokeMethod(this, [this, status]() {
            onConnectionStatusChanged(status);
        }, Qt::QueuedConnection);
    });

    // Регистрируем callback для телеметрии подвеса
    m_gimbal->setTelemetryCallback([this](const Telemetry& t) {
        QMetaObject::invokeMethod(this, [this, t]() {
            onGimbalTelemetryReceived(t);
        }, Qt::QueuedConnection);
    });

    // Подключаемся к сигналу телеметрии ArduPilot
    connect(m_mavlinkStream.get(), &MavlinkStream::telemetryUpdated,
            this, [] (const MavlinkTelemetry& telemetry) {
        // Обработка телеметрии ArduPilot
        static int logCounter = 0;
        if (++logCounter % 50 == 0) {
            qDebug() << "[ApplicationManager] ArduPilot Telemetry:"
                     << "pos=[" << telemetry.latitude_deg << "," << telemetry.longitude_deg << "]"
                     << "alt=" << telemetry.relative_altitude_m << "m"
                     << "att=[r:" << telemetry.roll_deg << " p:" << telemetry.pitch_deg << " y:" << telemetry.yaw_deg << "]"
                     << "speed=" << telemetry.speed_m_s << "m/s"
                     << "bat=" << telemetry.battery_percentage << "%"
                     << "armed=" << (telemetry.armed ? "YES" : "NO")
                     << "gps=" << telemetry.gps_fix_type << "(" << telemetry.gps_num_satellites << "sats)"
                     << "rc=" << (telemetry.rc_available ? "OK" : "NO")
                     << "mode=" << QString::fromStdString(telemetry.flight_mode);
        }
    });

    // Подключаемся к сигналу статуса вооружения
    connect(m_mavlinkStream.get(), &MavlinkStream::armedChanged,
            this, [] (bool armed) {
        qDebug() << "[ApplicationManager] Arming status changed:" << (armed ? "ARMED" : "DISARMED");
    });

    // Подключаем ArduPilot (UDP порт 14552)
    qDebug() << "[ApplicationManager] Connecting to ArduPilot on UDP 14552...";
    m_mavlinkStream->connect(14552);

    qDebug() << "[ApplicationManager] Gimbal components initialized";
}

void ApplicationManager::cleanupGimbalComponents() {
    // Останавливаем ControlStream
    stopControlStream();

    // Останавливаем опрос джойстика
    if (m_joystickHandler) {
        m_joystickHandler->stop();
    }

    // Отключаем ArduPilot
    if (m_mavlinkStream) {
        m_mavlinkStream->disconnect();
    }

    if (m_connectionTimeoutTimer) {
        m_connectionTimeoutTimer->stop();
    }

    // Очищаем указатели
    m_gimbal.reset();
    m_videoStream.reset();
    m_configManager.reset();
    m_keyboardHandler.reset();
    m_joystickHandler.reset();
    m_mavlinkStream.reset();
    m_controlStream = nullptr;

    qDebug() << "[ApplicationManager] Gimbal components cleaned up";
}

void ApplicationManager::startControlStream() {
    if (m_controlStream && !m_controlStream->isRunning()) {
        qDebug() << "[ApplicationManager] [CONTROL] Starting ControlStream at 10 Hz";
        m_controlStream->start();
    }
}

void ApplicationManager::stopControlStream() {
    if (m_controlStream && m_controlStream->isRunning()) {
        qDebug() << "[ApplicationManager] [CONTROL] Stopping ControlStream";
        m_controlStream->stop();
        ControlStream::reset();
    }
}

void ApplicationManager::onConnectionStatusChanged(ConnectionStatus status) {
    qDebug() << "[ApplicationManager] Connection status changed:"
             << static_cast<int>(status);

    switch (status) {
        case ConnectionStatus::TcpConnected:
        case ConnectionStatus::UdpConnected:
        case ConnectionStatus::SerialPortConnected: {
            m_isConnected = true;
            m_connectionPending = false;
            m_connectionTimeoutTimer->stop();
            qDebug() << "[ApplicationManager] Gimbal connected";

            // Включаем моторы
            if (m_gimbal) {
                m_gimbal->motorOn(true);
                qDebug() << "[ApplicationManager] Gimbal motors ON";
            }

            // Запускаем видеопоток
            std::string rtspUrl = VideoStream::buildRtspUrl(m_config);
            bool videoResult = m_videoStream->start(m_config);
            qDebug() << "[ApplicationManager] [VIDEO] Video stream started:" << (videoResult ? "YES" : "NO");

            if (!videoResult) {
                qCritical() << "[ApplicationManager] [VIDEO] Failed to start video stream";
            }

            // Запускаем ControlStream
            startControlStream();

            // Применяем текущее положение оси видео
            if (m_joystickHandler) {
                m_joystickHandler->applyCurrentVideoState();
            }

            emit gimbalConnectionChanged(true);
            break;
        }

        case ConnectionStatus::TcpDisconnected:
        case ConnectionStatus::UdpDisconnected:
        case ConnectionStatus::SerialPortDisconnected:
            m_isConnected = false;
            m_connectionPending = false;
            m_connectionTimeoutTimer->stop();
            qDebug() << "[ApplicationManager] Gimbal disconnected";
            emit gimbalConnectionChanged(false);
            break;

        default:
            break;
    }

    emit connectionStatusChanged(status);
}

void ApplicationManager::onGimbalTelemetryReceived(const Telemetry& telemetry) {
    // Обработка телеметрии от подвеса ViewLink
    static int logCounter = 0;
    if (++logCounter % 100 == 0) {
        qDebug() << "[ApplicationManager] Gimbal Telemetry:"
                 << "yaw=" << telemetry.yaw
                 << ", pitch=" << telemetry.pitch
                 << ", zoom=" << telemetry.zoomMagTimes;
    }
}

} // namespace gimbal
