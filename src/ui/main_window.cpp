#include "main_window.h"
#include "widgets/video/video_widget.h"
#include "widgets/transparent/transparent_widget.h"
#include "widgets/panels/telemetry_panel.h"
#include "gimbal/command_handler.h"

#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

MainWindow::MainWindow(gimbal::ApplicationManager* appManager, QWidget *parent)
    : QMainWindow(parent)
    , m_appManager(appManager)
{
    if (!m_appManager) {
        qCritical() << "MainWindow: ApplicationManager is null!";
        return;
    }

    setWindowTitle("Viewpro Gimbal Control");

    // Убираем стандартную рамку окна
    setWindowFlags(Qt::FramelessWindowHint);

    // Принимаем фокус клавиатуры
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

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
    connect(m_appManager->videoStream(), &gimbal::VideoStream::frameReady,
            this, &MainWindow::onFrameReady);
    connect(m_appManager->videoStream(), &gimbal::VideoStream::errorOccurred,
            this, &MainWindow::onVideoError);

    // Подключаем сигнал телеметрии ArduPilot для обновления панели
    connect(m_appManager->mavlinkStream(), &gimbal::MavlinkStream::telemetryUpdated,
            this, &MainWindow::onArduPilotTelemetryUpdated);

    // Устанавливаем фиксированный размер окна
    resize(960, 540);

    qDebug() << "MainWindow: initialized, window size:" << size();

    // Авто-подключение к подвесу при запуске
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "MainWindow: auto-connecting to gimbal...";
        m_appManager->connectToGimbal();
    });
}

MainWindow::~MainWindow() {
    // ApplicationManager очищается в main.cpp
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

void MainWindow::onTargetAcquire(const QPoint& clickPos)
{
    qDebug() << "=== MainWindow: onTargetAcquire ===";
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        QSize videoSize = videoWidget->videoSize();
        qDebug() << "  -> Video size:" << videoSize << ", click at:" << clickPos;
        qDebug() << "  -> Отправка команды на ЗАХВАТ ЦЕЛИ (target acquire command)";
        
        if (m_appManager && m_appManager->isConnected()) {
            gimbal::CommandHandler::targetAcquire(clickPos, videoSize.width(), videoSize.height());
        }
    }
}

void MainWindow::onTargetCancel()
{
    qDebug() << "=== MainWindow: onTargetCancel ===";
    qDebug() << "  -> Отправка команды на ОТМЕНУ ЗАХВАТА (target cancel command)";
    
    if (m_appManager && m_appManager->isConnected()) {
        gimbal::CommandHandler::targetCancel();
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
    
    // Получаем панель телеметрии и показываем/скрываем её
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        if (auto* transparentWidget = videoWidget->transparentWidget()) {
            if (auto* telemetryPanel = transparentWidget->telemetryPanel()) {
                if (active) {
                    telemetryPanel->showPanel();
                } else {
                    telemetryPanel->hidePanel();
                }
            }
        }
    }
    
    // Обработка через ApplicationManager
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

// === Управление ControlStream ===

void MainWindow::startControlStream() {
    if (m_appManager && m_appManager->controlStream()) {
        auto* controlStream = m_appManager->controlStream();
        if (!controlStream->isRunning()) {
            qDebug() << "MainWindow: [CONTROL] starting ControlStream at 10 Hz";
            controlStream->start();
        }
    }
}

void MainWindow::stopControlStream() {
    if (m_appManager && m_appManager->controlStream()) {
        auto* controlStream = m_appManager->controlStream();
        if (controlStream->isRunning()) {
            qDebug() << "MainWindow: [CONTROL] stopping ControlStream";
            controlStream->stop();
            gimbal::ControlStream::reset();
        }
    }
}

// === Обработка клавиатуры ===

bool MainWindow::event(QEvent* event) {
    // Перехватываем события клавиатуры даже если фокус на другом виджете
    if (m_appManager && m_appManager->isConnected() && 
        m_appManager->controlStream() && m_appManager->keyboardHandler()) {
        
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (m_appManager->keyboardHandler()->handleKeyPress(keyEvent, m_appManager->controlStream())) {
                return true;  // Событие обработано
            }
        }
        else if (event->type() == QEvent::KeyRelease) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (m_appManager->keyboardHandler()->handleKeyRelease(keyEvent, m_appManager->controlStream())) {
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

// === Обработка телеметрии ArduPilot ===

void MainWindow::onArduPilotTelemetryUpdated(const gimbal::MavlinkTelemetry& telemetry) {
    // Обновляем панель телеметрии
    if (auto* videoWidget = qobject_cast<VideoWidget*>(centralWidget())) {
        if (auto* transparentWidget = videoWidget->transparentWidget()) {
            if (auto* telemetryPanel = transparentWidget->telemetryPanel()) {
                telemetryPanel->updateTelemetry(telemetry);
            }
        }
    }
    
    // Обработка телеметрии от ArduPilot
    // Здесь можно обновлять UI элементы с данными телеметрии
    static int logCounter = 0;
    if (++logCounter % 50 == 0) {
        qDebug() << "[MainWindow] ArduPilot Telemetry:"
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
}

void MainWindow::onArmingStatusChanged(bool armed) {
    qDebug() << "[MainWindow] Arming status changed:" << (armed ? "ARMED" : "DISARMED");
    // Здесь можно обновлять UI (например, индикатор вооружения)
    // Статус выводится только при изменении
}

// === Обработка закрытия окна ===

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << "[MainWindow] Close event received, shutting down...";
    
    // Закрываем окно
    event->accept();
    
    qDebug() << "[MainWindow] Window closed";
}
