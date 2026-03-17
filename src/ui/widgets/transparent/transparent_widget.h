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
class DirectionButton;
class DirectionPanel;
class TelemetryPanel;
class FlightInfoWidget;

class TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransparentWidget(QWidget *parent = nullptr);
    ~TransparentWidget() override = default;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

    /**
     * @brief Получить указатель на панель телеметрии
     */
    TelemetryPanel* telemetryPanel() const { return m_telemetryPanel; }

    /**
     * @brief Получить указатель на панель полётной информации
     */
    FlightInfoWidget* flightInfoWidget() const { return m_flightInfoWidget; }

    /**
     * @brief Получить указатель на кнопку направления
     */
    DirectionButton* directionButton() const { return m_directionButton; }

    /**
     * @brief Получить указатель на панель направления
     */
    DirectionPanel* directionPanel() const { return m_directionPanel; }

signals:
    // Режим фиксированного размера
    void targetAcquire(const QPoint& pos);  // Короткое нажатие - захват цели
    void targetCancel();                    // Долгое нажатие без движения - отмена захвата
    void startDrag();                       // Долгое нажатие с движением - начало перетаскивания
    void endDrag();                         // Конец перетаскивания

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
    void directionToggled(bool active);

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
    void setupTelemetryPanel();
    void updateTelemetryPanelPosition();
    void setupDirectionPanel();
    void updateDirectionPanelPosition();
    void setupFlightInfoWidget();
    void updateFlightInfoWidgetPosition();

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
    DirectionButton* m_directionButton = nullptr;

    // Панель телеметрии
    TelemetryPanel* m_telemetryPanel = nullptr;

    // Панель направления
    DirectionPanel* m_directionPanel = nullptr;

    // Панель полётной информации
    FlightInfoWidget* m_flightInfoWidget = nullptr;
};
