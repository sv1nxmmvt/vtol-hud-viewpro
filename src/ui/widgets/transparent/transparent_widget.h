#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>

class QWindow;

class TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransparentWidget(QWidget *parent = nullptr);
    ~TransparentWidget() override = default;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

signals:
    // Режим фиксированного размера
    void targetAcquire();      // Короткое нажатие - захват цели
    void targetCancel();       // Долгое нажатие без движения - отмена захвата
    void startDrag();          // Долгое нажатие с движением - начало перетаскивания
    void endDrag();            // Конец перетаскивания

    // Полноэкранный режим
    void gimbalMove(const QPoint& delta);  // Управление гимбалом

protected:
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
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
};
