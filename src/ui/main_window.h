#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QTimer>
#include <memory>

#include "gimbal/gimbal.h"
#include "gimbal/video_stream.h"
#include "gimbal/config_manager.h"
#include "gimbal/control_stream.h"
#include "gimbal/keyboard_handler.h"
#include "gimbal/joystick_handler.h"
#include "gimbal/command_handler.h"
#include "gimbal/telemetry_stream.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

private slots:
    // Обработка сигналов TransparentWidget
    void onTargetAcquire();      // Захват цели (короткое нажатие)
    void onTargetCancel();       // Отмена захвата (долгое нажатие без движения)
    void onStartDrag();          // Начало перетаскивания (режим окна)
    void onEndDrag();            // Конец перетаскивания (режим окна)

    // Обработка сигналов кнопок управления окном
    void onCloseClicked();
    void onHideClicked();
    void onResizeClicked();

    // Обработка сигналов кнопок управления подвесом
    void onTelemetryToggled(bool active);
    void onControlToggled(bool active);

    // Обработка видеопотока
    void onFrameReady(const QImage& frame);
    void onVideoError(const QString& error);

    // Обработка статуса подключения
    void onConnectionStatusChanged(gimbal::ConnectionStatus status);

    // Обработка телеметрии
    void onTelemetryUpdated(const gimbal::Telemetry& telemetry);

private:
    bool m_fullscreen = false;
    QRect m_normalGeometry;

    // Компоненты для работы с подвесом
    std::shared_ptr<gimbal::Gimbal> m_gimbal;
    std::unique_ptr<gimbal::VideoStream> m_videoStream;
    std::unique_ptr<gimbal::ConfigManager> m_configManager;
    std::unique_ptr<gimbal::KeyboardHandler> m_keyboardHandler;
    std::unique_ptr<gimbal::JoystickHandler> m_joystickHandler;
    std::unique_ptr<gimbal::TelemetryStream> m_telemetryStream;
    gimbal::ControlStream* m_controlStream;  // Не владеющий указатель (singleton)
    gimbal::ConnectionConfig m_config;

    bool m_isConnected = false;
    bool m_connectionPending = false;
    QTimer* m_connectionTimeoutTimer = nullptr;

    // Инициализация компонентов подвеса
    void initGimbalComponents();
    void cleanupGimbalComponents();

    // Подключение/отключение
    bool connectToGimbal();
    void disconnectFromGimbal();

    // Управление ControlStream
    void startControlStream();
    void stopControlStream();

protected:
    // Обработка клавиатуры для управления подвесом
    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};
