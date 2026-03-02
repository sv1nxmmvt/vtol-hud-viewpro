#pragma once

#include <QMainWindow>
#include <QPoint>
#include <memory>

#include "gimbal/gimbal.h"
#include "gimbal/video_stream.h"
#include "gimbal/config_manager.h"

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
    void onGimbalMove(const QPoint& delta);  // Управление гимбалом (полноэкранный режим)

    // Обработка сигналов кнопок управления окном
    void onCloseClicked();
    void onHideClicked();
    void onResizeClicked();

    // Обработка сигналов кнопок управления подвесом
    void onConnectionToggled(bool active);
    void onTelemetryToggled(bool active);
    void onControlToggled(bool active);

    // Обработка видеопотока
    void onFrameReady(const QImage& frame);
    void onVideoError(const QString& error);

    // Обработка статуса подключения
    void onConnectionStatusChanged(gimbal::ConnectionStatus status);

private:
    bool m_fullscreen = false;
    QRect m_normalGeometry;

    // Компоненты для работы с подвесом
    std::unique_ptr<gimbal::Gimbal> m_gimbal;
    std::unique_ptr<gimbal::VideoStream> m_videoStream;
    std::unique_ptr<gimbal::ConfigManager> m_configManager;
    gimbal::ConnectionConfig m_config;

    bool m_isConnected = false;

    // Инициализация компонентов подвеса
    void initGimbalComponents();
    void cleanupGimbalComponents();

    // Подключение/отключение
    bool connectToGimbal();
    void disconnectFromGimbal();
};
