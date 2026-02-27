#pragma once

#include <QMainWindow>
#include <QPoint>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

private slots:
    // Обработка сигналов TransparentWidget
    void onTargetAcquire();      // Захват цели (короткое нажатие)
    void onTargetCancel();       // Отмена захвата (долгое нажатие без движения)
    void onStartDrag();          // Начало перетаскивания (режим окна)
    void onEndDrag();            // Конец перетаскивания (режим окна)
    void onGimbalMove(const QPoint& delta);  // Управление гимбалом (полноэкранный режим)

private:
    bool m_fullscreen = false;
    QRect m_normalGeometry;
};
