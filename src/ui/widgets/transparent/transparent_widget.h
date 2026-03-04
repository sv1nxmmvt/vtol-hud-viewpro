#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>

class QWindow;
class CloseWidget;
class HideWidget;
class ResizeWidget;
class TelemetryButton;
class ControlButton;
class LaserRangeWidget;

class TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransparentWidget(QWidget *parent = nullptr);
    ~TransparentWidget() override = default;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

    /**
     * @brief Обновить расстояние от лазера
     * @param distance Расстояние в метрах
     */
    void setLaserDistance(double distance);

signals:
    // Режим фиксированного размера
    void targetAcquire();      // Короткое нажатие - захват цели
    void targetCancel();       // Долгое нажатие без движения - отмена захвата
    void startDrag();          // Долгое нажатие с движением - начало перетаскивания
    void endDrag();            // Конец перетаскивания

    // Полноэкранный режим
    void gimbalMove(const QPoint& delta);  // Управление гимбалом
    void gimbalStop();                     // Остановка гимбала

    // Сигналы кнопок управления окном
    void closeClicked();
    void hideClicked();
    void resizeClicked();

    // Сигналы кнопок управления подвесом
    void telemetryToggled(bool active);
    void controlToggled(bool active);

protected:
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupWindowButtons();
    void updateWindowButtonsPosition();
    void setupGimbalButtons();
    void updateGimbalButtonsPosition();

    bool m_fullscreen = false;
    QTimer* m_pressTimer = nullptr;
    QPoint m_pressPosition;
    QPoint m_windowPos;
    QWidget* m_topLevelWidget = nullptr;
    QWindow* m_topLevelWindow = nullptr;
    bool m_isDragging = false;
    bool m_timerExpired = false;
    bool m_mouseMoved = false;
    bool m_dragStarted = false;
    bool m_pressIgnored = false;

    // Кнопки управления окном
    CloseWidget* m_closeButton = nullptr;
    HideWidget* m_hideButton = nullptr;
    ResizeWidget* m_resizeButton = nullptr;

    // Кнопки управления подвесом
    TelemetryButton* m_telemetryButton = nullptr;
    ControlButton* m_controlButton = nullptr;

    // Виджет дальномера
    LaserRangeWidget* m_laserRangeWidget = nullptr;
};
