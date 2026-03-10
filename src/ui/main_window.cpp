#include "main_window.h"
#include "widgets/video/video_widget.h"
#include "widgets/transparent/transparent_widget.h"

#include "gimbal/gimbal.h"
#include "gimbal/video_stream.h"
#include "gimbal/config_manager.h"
#include "gimbal/keyboard_handler.h"

#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_controlStream(nullptr)
{
    setWindowTitle("Viewpro Gimbal Control");

    // Убираем стандартную рамку окна
    setWindowFlags(Qt::FramelessWindowHint);

    // Принимаем фокус клавиатуры
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    // Создаём таймер таймаута подключения
    m_connectionTimeoutTimer = new QTimer(this);
    m_connectionTimeoutTimer->setSingleShot(true);
    connect(m_connectionTimeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_connectionPending) {
            qWarning() << "MainWindow: connection timeout - gimbal not responding";
            m_connectionPending = false;
            // Продолжаем работу без подключения
        }
    });

    // Инициализируем компоненты подвеса
    initGimbalComponents();

    // Виджет видео как центральный элемент
    auto* videoWidget = new VideoWidget(this);
    setCentralWidget(videoWidget);

    // Подключаем сигналы прозрачного виджета
    auto* transparentWidget = videoWidget->transparentWidget();

    // Режим фиксированного размера
    connect(transparentWidget, &TransparentWidget::targetAcquire, this, &MainWindow::onTargetAcquire);
    connect(transparentWidget, &TransparentWidget::targetCancel, this, &MainWindow::onTargetCancel);
    connect(transparentWidget, &TransparentWidget::startDrag, this, &MainWindow::onStartDrag);
    connect(transparentWidget, &TransparentWidget::endDrag, this, &MainWindow::onEndDrag);

    // Кнопки управления окном
    connect(transparentWidget, &TransparentWidget::closeClicked, this, &MainWindow::onCloseClicked);
    connect(transparentWidget, &TransparentWidget::hideClicked, this, &MainWindow::onHideClicked);
    connect(transparentWidget, &TransparentWidget::resizeClicked, this, &MainWindow::onResizeClicked);

    // Кнопки управления подвесом
    connect(transparentWidget, &TransparentWidget::telemetryToggled, this, &MainWindow::onTelemetryToggled);
    connect(transparentWidget, &TransparentWidget::controlToggled, this, &MainWindow::onControlToggled);

    // Подключаем сигналы видеопотока
    connect(m_videoStream.get(), &gimbal::VideoStream::frameReady,
            this, &MainWindow::onFrameReady);
    connect(m_videoStream.get(), &gimbal::VideoStream::errorOccurred,
            this, &MainWindow::onVideoError);

    // Устанавливаем фиксированный размер окна
    resize(960, 540);

    qDebug() << "MainWindow: initialized, window size:" << size();

    // Авто-подключение к подвесу при запуске
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "MainWindow: auto-connecting to gimbal...";
        connectToGimbal();
    });
}

MainWindow::~MainWindow() {
    disconnectFromGimbal();
    cleanupGimbalComponents();
}

void MainWindow::initGimbalComponents() {
    // Инициализируем SDK
    if (!gimbal::Gimbal::init()) {
        qCritical() << "MainWindow: failed to initialize Gimbal SDK";
        return;
    }
    qDebug() << "MainWindow: Gimbal SDK initialized, version:" << gimbal::Gimbal::getSdkVersion();

    // Создаём компоненты
    m_gimbal = std::make_shared<gimbal::Gimbal>();
    m_videoStream = std::make_unique<gimbal::VideoStream>();
    m_configManager = std::make_unique<gimbal::ConfigManager>();

    // Инициализируем ControlStream (singleton)
    m_controlStream = &gimbal::ControlStream::instance(*m_gimbal);
    m_controlStream->setFrequency(10);  // 10 Гц для отзывчивого управления

    // Инициализируем обработчик клавиатуры
    m_keyboardHandler = std::make_unique<gimbal::KeyboardHandler>(this);

    // Инициализируем обработчик джойстика и запускаем опрос
    m_joystickHandler = std::make_unique<gimbal::JoystickHandler>(this);
    m_joystickHandler->start();

    // Инициализируем CommandHandler для отправки команд
    gimbal::CommandHandler::init(m_gimbal);

    // Загружаем конфигурацию
    auto configOpt = m_configManager->load();
    if (configOpt) {
        m_config = *configOpt;
        qDebug() << "MainWindow: configuration loaded:"
                 << QString::fromStdString(m_config.getTypeString())
                 << ", IP:" << QString::fromStdString(m_config.ip)
                 << ", port:" << m_config.port
                 << ", video port:" << m_config.videoPort;
    } else {
        m_config = gimbal::ConfigManager::getDefaultConfig();
        qDebug() << "MainWindow: using default configuration";
    }

    // Настраиваем Keep Alive Interval для более отзывчивой телеметрии
    // 500 мс по умолчанию - не меняем, чтобы не создавать лишнюю нагрузку
    // m_gimbal->setKeepAliveInterval(200);  // Закомментировано - используем дефолтное значение

    // Регистрируем callback для статуса подключения
    m_gimbal->setConnectionCallback([this](gimbal::ConnectionStatus status) {
        QMetaObject::invokeMethod(this, [this, status]() {
            onConnectionStatusChanged(status);
        }, Qt::QueuedConnection);
    });

    // Регистрируем callback для телеметрии
    m_gimbal->setTelemetryCallback([this](const gimbal::Telemetry& t) {
        QMetaObject::invokeMethod(this, [this, t]() {
            // Обработка телеметрии
            qDebug() << "Telemetry: yaw=" << t.yaw
                     << ", pitch=" << t.pitch
                     << ", zoom=" << t.zoomMagTimes;
        }, Qt::QueuedConnection);
    });

    qDebug() << "MainWindow: gimbal components initialized";
}

void MainWindow::cleanupGimbalComponents() {
    // Останавливаем ControlStream
    stopControlStream();

    // Останавливаем опрос джойстика
    if (m_joystickHandler) {
        m_joystickHandler->stop();
    }

    if (m_connectionTimeoutTimer) {
        m_connectionTimeoutTimer->stop();
    }

    // Сначала отключаемся от подвеса, потом освобождаем ресурсы
    if (m_gimbal) {
        m_gimbal->stop();
        m_gimbal->motorOn(false);
        m_gimbal->disconnect();
    }

    m_gimbal.reset();
    m_videoStream.reset();
    m_configManager.reset();
    gimbal::Gimbal::shutdown();
    qDebug() << "MainWindow: gimbal components cleaned up";
}

bool MainWindow::connectToGimbal() {
    if (m_connectionPending) {
        qDebug() << "MainWindow: connection already in progress";
        return false;
    }

    qDebug() << "MainWindow: === CONNECTING TO GIMBAL ===";

    if (!m_gimbal) {
        qCritical() << "MainWindow: [ERROR] gimbal not initialized";
        return false;
    }

    qDebug() << "MainWindow: [CONFIG] target:" << QString::fromStdString(m_config.ip)
             << ":" << m_config.port
             << ", type:" << QString::fromStdString(m_config.getTypeString())
             << ", videoPort:" << m_config.videoPort;

    // Устанавливаем таймаут подключения (3 секунды)
    m_connectionPending = true;
    m_connectionTimeoutTimer->setInterval(3000);
    m_connectionTimeoutTimer->start();

    // Подключаемся к подвесу
    qDebug() << "MainWindow: [GIMBAL] calling connect()...";
    bool connectResult = m_gimbal->connect(m_config);
    qDebug() << "MainWindow: [GIMBAL] connect() returned:" << (connectResult ? "true" : "false");

    if (!connectResult) {
        qCritical() << "MainWindow: [ERROR] gimbal connect() failed";
        m_connectionPending = false;
        m_connectionTimeoutTimer->stop();
        return false;
    }

    qDebug() << "MainWindow: === CONNECTION INITIATED (waiting for status) ===";
    return true;
}

void MainWindow::disconnectFromGimbal() {
    if (!m_isConnected && !m_connectionPending) {
        return;
    }

    qDebug() << "MainWindow: disconnecting from gimbal";

    // Сначала останавливаем ControlStream чтобы не отправлял команды
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

    // Очищаем отображение видео
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        videoWidget->clearDisplay();
    }

    m_isConnected = false;
    m_connectionPending = false;
    qDebug() << "MainWindow: disconnected from gimbal";
}

void MainWindow::setFullscreen(bool fullscreen)
{
    if (m_fullscreen == fullscreen) {
        return;
    }

    m_fullscreen = fullscreen;
    qDebug() << "MainWindow: setFullscreen" << fullscreen;

    // Обновляем состояние прозрачного виджета
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        if (auto* transparentWidget = videoWidget->transparentWidget()) {
            transparentWidget->setFullscreen(fullscreen);
        }
    }

    if (fullscreen) {
        m_normalGeometry = geometry();
        showFullScreen();
        qDebug() << "MainWindow: entered fullscreen mode";
    } else {
        showNormal();
        setGeometry(m_normalGeometry);
        qDebug() << "MainWindow: exited fullscreen mode, restored to:" << m_normalGeometry;
    }
}

// === Режим фиксированного размера ===

void MainWindow::onTargetAcquire()
{
    qDebug() << "=== MainWindow: onTargetAcquire ===";
    qDebug() << "  -> Отправка команды на ЗАХВАТ ЦЕЛИ (target acquire command)";
    if (m_isConnected && m_gimbal) {
        // TODO: Здесь будет логика отправки команды на подвес для захвата цели
        // Например: m_gimbal->startTrack();
    }
}

void MainWindow::onTargetCancel()
{
    qDebug() << "=== MainWindow: onTargetCancel ===";
    qDebug() << "  -> Отправка команды на ОТМЕНУ ЗАХВАТА (target cancel command)";
    if (m_isConnected && m_gimbal) {
        // TODO: Здесь будет логика отмены захвата
        // Например: m_gimbal->stopTrack();
    }
}

void MainWindow::onStartDrag()
{
    qDebug() << "=== MainWindow: onStartDrag ===";
    qDebug() << "  -> Начало перетаскивания окна (window drag started)";
}

void MainWindow::onEndDrag()
{
    qDebug() << "=== MainWindow: onEndDrag ===";
    qDebug() << "  -> Завершение перетаскивания окна (window drag ended)";
}

// === Кнопки управления окном ===

void MainWindow::onCloseClicked()
{
    qDebug() << "=== MainWindow: onCloseClicked ===";
    qDebug() << "  -> Закрытие приложения (closing application)";

    // Отключаемся от подвеса перед закрытием
    disconnectFromGimbal();
    close();
}

void MainWindow::onHideClicked()
{
    qDebug() << "=== MainWindow: onHideClicked ===";
    qDebug() << "  -> Скрытие окна в панель задач (minimizing to taskbar)";
    showMinimized();
}

void MainWindow::onResizeClicked()
{
    qDebug() << "=== MainWindow: onResizeClicked ===";
    qDebug() << "  -> Переключение режима окна (toggling window mode)";
    setFullscreen(!m_fullscreen);
}

// === Кнопки управления подвесом ===

void MainWindow::onTelemetryToggled(bool active)
{
    qDebug() << "=== MainWindow: onTelemetryToggled ===";
    qDebug() << "  -> Телеметрия (telemetry):" << (active ? "ON" : "OFF");

    if (active && m_isConnected && m_gimbal) {
        // Включаем получение телеметрии
        m_gimbal->setTelemetryCallback([this](const gimbal::Telemetry& telemetry) {
            // Обработка телеметрии
            qDebug() << "Telemetry: yaw=" << telemetry.yaw
                     << ", pitch=" << telemetry.pitch
                     << ", zoom=" << telemetry.zoomMagTimes;
        });
    }
}

void MainWindow::onControlToggled(bool active)
{
    qDebug() << "=== MainWindow: onControlToggled ===";
    qDebug() << "  -> Управление подвесом (gimbal control):" << (active ? "ON" : "OFF");
    // TODO: Здесь будет логика включения/выключения управления подвесом
}

// === Обработка видеопотока ===

void MainWindow::onFrameReady(const QImage& frame)
{
    if (frame.isNull()) {
        return;
    }

    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        videoWidget->displayFrame(frame);
    }
}

void MainWindow::onVideoError(const QString& error)
{
    qCritical() << "MainWindow: [VIDEO ERROR]" << error;
    // TODO: Показать пользователю сообщение об ошибке
}

// === Обработка статуса подключения ===

void MainWindow::onConnectionStatusChanged(gimbal::ConnectionStatus status)
{
    qDebug() << "MainWindow: connection status changed:"
             << static_cast<int>(status);

    switch (status) {
        case gimbal::ConnectionStatus::TcpConnected:
        case gimbal::ConnectionStatus::UdpConnected:
        case gimbal::ConnectionStatus::SerialPortConnected: {
            m_isConnected = true;
            m_connectionPending = false;
            m_connectionTimeoutTimer->stop();
            qDebug() << "MainWindow: gimbal connected";

            // Включаем моторы
            if (m_gimbal) {
                m_gimbal->motorOn(true);
                qDebug() << "MainWindow: gimbal motors ON";
            }

            // Запускаем видеопоток
            qDebug() << "MainWindow: [VIDEO] starting video stream...";
            std::string rtspUrl = gimbal::VideoStream::buildRtspUrl(m_config);
            qDebug() << "MainWindow: [VIDEO] RTSP URL:" << QString::fromStdString(rtspUrl);

            bool videoResult = m_videoStream->start(m_config);
            qDebug() << "MainWindow: [VIDEO] start() returned:" << (videoResult ? "true" : "false");
            qDebug() << "MainWindow: [VIDEO] isPlaying:" << m_videoStream->isPlaying();

            if (!videoResult) {
                qCritical() << "MainWindow: [VIDEO] failed to start video stream";
            } else {
                qDebug() << "MainWindow: [VIDEO] video stream started successfully";
            }

            // Запускаем ControlStream для отправки команд
            startControlStream();

            // Применяем текущее положение оси видео
            if (m_joystickHandler) {
                m_joystickHandler->applyCurrentVideoState();
            }
            break;
        }

        case gimbal::ConnectionStatus::TcpDisconnected:
        case gimbal::ConnectionStatus::UdpDisconnected:
        case gimbal::ConnectionStatus::SerialPortDisconnected:
            m_isConnected = false;
            m_connectionPending = false;
            m_connectionTimeoutTimer->stop();
            qDebug() << "MainWindow: gimbal disconnected";
            break;

        default:
            break;
    }
}

// === Управление ControlStream ===

void MainWindow::startControlStream() {
    if (m_controlStream && !m_controlStream->isRunning()) {
        qDebug() << "MainWindow: [CONTROL] starting ControlStream at 10 Hz";
        m_controlStream->start();
    }
}

void MainWindow::stopControlStream() {
    if (m_controlStream && m_controlStream->isRunning()) {
        qDebug() << "MainWindow: [CONTROL] stopping ControlStream";
        m_controlStream->stop();
        // Сбрасываем управляющие сигналы
        gimbal::ControlStream::reset();
    }
}

// === Обработка клавиатуры ===

bool MainWindow::event(QEvent* event) {
    // Перехватываем события клавиатуры даже если фокус на другом виджете
    if (m_isConnected && m_controlStream && m_keyboardHandler) {
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (m_keyboardHandler->handleKeyPress(keyEvent, m_controlStream)) {
                return true;  // Событие обработано
            }
        }
        else if (event->type() == QEvent::KeyRelease) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (m_keyboardHandler->handleKeyRelease(keyEvent, m_controlStream)) {
                return true;  // Событие обработано
            }
        }
    }
    return QMainWindow::event(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // Обработка через event() перехватывает клавиши, здесь только для fallback
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    // Обработка через event() перехватывает клавиши, здесь только для fallback
    QMainWindow::keyReleaseEvent(event);
}
