#include "main_window.h"
#include "widgets/video/video_widget.h"
#include "widgets/transparent/transparent_widget.h"
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
    
    // Кнопки управления окном
    connect(transparentWidget, &TransparentWidget::closeClicked, this, &MainWindow::onCloseClicked);
    connect(transparentWidget, &TransparentWidget::hideClicked, this, &MainWindow::onHideClicked);
    connect(transparentWidget, &TransparentWidget::resizeClicked, this, &MainWindow::onResizeClicked);
    
    // Устанавливаем фиксированный размер окна
    resize(960, 540);
    
    qDebug() << "MainWindow: initialized, window size:" << size();
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
    // 1.1) Короткое нажатие (<0.2с) - захват цели
    qDebug() << "=== MainWindow: onTargetAcquire ===";
    qDebug() << "  -> Отправка команды на ЗАХВАТ ЦЕЛИ (target acquire command)";
    // TODO: Здесь будет логика отправки команды на подвес для захвата цели
    // Например: gimbal->sendTargetAcquireCommand();
}

void MainWindow::onTargetCancel()
{
    // 1.2) Долгое нажатие без движения (>0.2с, мышь не двигалась) - отмена захвата
    qDebug() << "=== MainWindow: onTargetCancel ===";
    qDebug() << "  -> Отправка команды на ОТМЕНУ ЗАХВАТА (target cancel command)";
    // TODO: Здесь будет логика отмены захвата цели
    // Например: gimbal->sendTargetCancelCommand();
}

void MainWindow::onStartDrag()
{
    // Начало перетаскивания окна
    qDebug() << "=== MainWindow: onStartDrag ===";
    qDebug() << "  -> Начало перетаскивания окна (window drag started)";
}

void MainWindow::onEndDrag()
{
    // Завершение перетаскивания окна
    qDebug() << "=== MainWindow: onEndDrag ===";
    qDebug() << "  -> Завершение перетаскивания окна (window drag ended)";
}

// === Полноэкранный режим ===

void MainWindow::onGimbalMove(const QPoint& delta)
{
    // 2.3) Долгое нажатие с движением в fullscreen - управление гимбалом
    static int moveCount = 0;
    moveCount++;
    
    qDebug() << "=== MainWindow: onGimbalMove #" << moveCount << "===";
    qDebug() << "  -> Управление ПОДВЕСОМ (gimbal control)";
    qDebug() << "  -> Delta:" << delta;
    qDebug() << "  -> Yaw (горизонталь):" << delta.x() << "units";
    qDebug() << "  -> Pitch (вертикаль):" << delta.y() << "units";
    // TODO: Здесь будет логика отправки команд на подвес
    // Например:
    // gimbal->moveYaw(delta.x() * sensitivity);
    // gimbal->movePitch(delta.y() * sensitivity);
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
