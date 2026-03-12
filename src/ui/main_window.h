#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QTimer>
#include <memory>

#include "gimbal/application_manager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(gimbal::ApplicationManager* appManager, QWidget *parent = nullptr);
    ~MainWindow() override;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

private slots:
    // Обработка сигналов TransparentWidget
    void onTargetAcquire(const QPoint& clickPos);  // Захват цели (короткое нажатие)
    void onTargetCancel();                         // Отмена захвата (долгое нажатие без движения)
    void onStartDrag();                            // Начало перетаскивания (режим окна)
    void onEndDrag();                              // Конец перетаскивания (режим окна)

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

    // Обработка телеметрии ArduPilot
    void onArduPilotTelemetryUpdated(const gimbal::MavlinkTelemetry& telemetry);

    // Обработка статуса вооружения
    void onArmingStatusChanged(bool armed);

private:
    bool m_fullscreen = false;
    QRect m_normalGeometry;

    // Менеджер приложения (не владеем)
    gimbal::ApplicationManager* m_appManager = nullptr;

    // Управление ControlStream
    void startControlStream();
    void stopControlStream();

protected:
    // Обработка клавиатуры для управления подвесом
    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    
    // Обработка закрытия окна
    void closeEvent(QCloseEvent* event) override;
};
