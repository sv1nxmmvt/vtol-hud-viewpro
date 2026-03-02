#include "main_window.h"
#include "widgets/video/video_widget.h"
#include "widgets/transparent/transparent_widget.h"

#include "gimbal/gimbal.h"
#include "gimbal/video_stream.h"
#include "gimbal/config_manager.h"
#include "gimbal/gimbal_config.h"

#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Viewpro Gimbal Control");

    // Убираем стандартную рамку окна
    setWindowFlags(Qt::FramelessWindowHint);

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

    // Полноэкранный режим
    connect(transparentWidget, &TransparentWidget::gimbalMove, this, &MainWindow::onGimbalMove);
    connect(transparentWidget, &TransparentWidget::gimbalStop, this, &MainWindow::onGimbalStop);

    // Кнопки управления окном
    connect(transparentWidget, &TransparentWidget::closeClicked, this, &MainWindow::onCloseClicked);
    connect(transparentWidget, &TransparentWidget::hideClicked, this, &MainWindow::onHideClicked);
    connect(transparentWidget, &TransparentWidget::resizeClicked, this, &MainWindow::onResizeClicked);

    // Кнопки управления подвесом
    connect(transparentWidget, &TransparentWidget::connectionToggled, this, &MainWindow::onConnectionToggled);
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
    m_gimbal = std::make_unique<gimbal::Gimbal>();
    m_videoStream = std::make_unique<gimbal::VideoStream>();
    m_configManager = std::make_unique<gimbal::ConfigManager>();

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

    // Регистрируем callback для статуса подключения
    m_gimbal->setConnectionCallback([this](gimbal::ConnectionStatus status) {
        QMetaObject::invokeMethod(this, [this, status]() {
            onConnectionStatusChanged(status);
        }, Qt::QueuedConnection);
    });

    qDebug() << "MainWindow: gimbal components initialized";
}

void MainWindow::cleanupGimbalComponents() {
    m_gimbal.reset();
    m_videoStream.reset();
    m_configManager.reset();
    gimbal::Gimbal::shutdown();
    qDebug() << "MainWindow: gimbal components cleaned up";
}

bool MainWindow::connectToGimbal() {
    qDebug() << "MainWindow: === CONNECTING TO GIMBAL ===";
    
    if (!m_gimbal) {
        qCritical() << "MainWindow: [ERROR] gimbal not initialized";
        return false;
    }

    qDebug() << "MainWindow: [CONFIG] target:" << QString::fromStdString(m_config.ip) 
             << ":" << m_config.port
             << ", type:" << QString::fromStdString(m_config.getTypeString())
             << ", videoPort:" << m_config.videoPort;

    // Подключаемся к подвесу
    qDebug() << "MainWindow: [GIMBAL] calling connect()...";
    bool connectResult = m_gimbal->connect(m_config);
    qDebug() << "MainWindow: [GIMBAL] connect() returned:" << (connectResult ? "true" : "false");
    
    if (!connectResult) {
        qCritical() << "MainWindow: [ERROR] gimbal connect() failed";
        return false;
    }

    // Запускаем видеопоток с небольшой задержкой
    QTimer::singleShot(500, this, [this]() {
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
    });

    m_isConnected = true;
    qDebug() << "MainWindow: === CONNECTION INITIATED ===";
    return true;
}

void MainWindow::disconnectFromGimbal() {
    if (!m_isConnected) {
        return;
    }

    qDebug() << "MainWindow: disconnecting from gimbal";

    // Останавливаем видеопоток
    if (m_videoStream) {
        m_videoStream->stop();
    }

    // Отключаемся от подвеса
    if (m_gimbal) {
        m_gimbal->disconnect();
    }

    // Очищаем отображение видео
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        videoWidget->clearDisplay();
    }

    m_isConnected = false;
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
        // TODO: Реализовать команду захвата цели
        // Например: m_gimbal->startTrack();
    }
}

void MainWindow::onTargetCancel()
{
    qDebug() << "=== MainWindow: onTargetCancel ===";
    qDebug() << "  -> Отправка команды на ОТМЕНУ ЗАХВАТА (target cancel command)";
    if (m_isConnected && m_gimbal) {
        // TODO: Реализовать команду отмены захвата
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

// === Полноэкранный режим ===

void MainWindow::onGimbalMove(const QPoint& delta)
{
    if (!m_isConnected || !m_gimbal) {
        return;
    }

    // 2.3) Долгое нажатие с движением в fullscreen - управление гимбалом
    static int moveCount = 0;
    moveCount++;

    qDebug() << "=== MainWindow: onGimbalMove #" << moveCount << "===";
    qDebug() << "  -> Управление ПОДВЕСОМ (gimbal control)";
    qDebug() << "  -> Delta:" << delta;
    qDebug() << "  -> Yaw (горизонталь):" << delta.x() << "units";
    qDebug() << "  -> Pitch (вертикаль):" << delta.y() << "units";

    // Преобразуем дельту в скорость (чувствительность можно настроить)
    const int sensitivity = 10;
    int yawSpeed = delta.x() * sensitivity;
    int pitchSpeed = -delta.y() * sensitivity;   // Инвертируем для естественного управления

    // Ограничиваем скорость максимально допустимыми значениями
    const int maxSpeed = 2000;  // VLK_MAX_YAW_SPEED / VLK_MAX_PITCH_SPEED
    yawSpeed = qBound(-maxSpeed, yawSpeed, maxSpeed);
    pitchSpeed = qBound(-maxSpeed, pitchSpeed, maxSpeed);

    m_gimbal->move(yawSpeed, pitchSpeed);
}

void MainWindow::onGimbalStop()
{
    if (!m_isConnected || !m_gimbal) {
        return;
    }

    qDebug() << "=== MainWindow: onGimbalStop ===";
    qDebug() << "  -> Остановка ПОДВЕСА (gimbal stop)";
    m_gimbal->stop();
}

// === Кнопки управления окном ===

void MainWindow::onCloseClicked()
{
    qDebug() << "=== MainWindow: onCloseClicked ===";
    qDebug() << "  -> Закрытие приложения (closing application)";
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

void MainWindow::onConnectionToggled(bool active)
{
    qDebug() << "=== MainWindow: onConnectionToggled ===";
    qDebug() << "  -> Подключение к подвесу (connection to gimbal):" << (active ? "ON" : "OFF");

    if (active) {
        if (!connectToGimbal()) {
            qCritical() << "MainWindow: connection failed";
            // Сбрасываем состояние кнопки в случае ошибки
            // Это должно обрабатываться на уровне UI
        }
    } else {
        disconnectFromGimbal();
    }
}

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
        qWarning() << "MainWindow: [VIDEO] received null frame";
        return;
    }
    
    static int frameCount = 0;
    frameCount++;
    
    if (frameCount <= 10 || frameCount % 30 == 0) {
        qDebug() << "MainWindow: [VIDEO] frame #" << frameCount 
                 << ", size:" << frame.size()
                 << ", format:" << frame.format()
                 << ", depth:" << frame.depth();
    }
    
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        videoWidget->displayFrame(frame);
        if (frameCount <= 5) {
            qDebug() << "MainWindow: [VIDEO] frame displayed on widget";
        }
    } else {
        qWarning() << "MainWindow: [VIDEO] videoWidget is null or not VideoWidget";
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
        case gimbal::ConnectionStatus::SerialPortConnected:
            m_isConnected = true;
            qDebug() << "MainWindow: gimbal connected";
            break;
            
        case gimbal::ConnectionStatus::TcpDisconnected:
        case gimbal::ConnectionStatus::UdpDisconnected:
        case gimbal::ConnectionStatus::SerialPortDisconnected:
            m_isConnected = false;
            qDebug() << "MainWindow: gimbal disconnected";
            break;
            
        default:
            break;
    }
}
