#include "transparent_widget.h"
#include "../titles/close_widget.h"
#include "../titles/hide_widget.h"
#include "../titles/resize_widget.h"
#include "../buttons/connection_button.h"
#include "../buttons/telemetry_button.h"
#include "../buttons/control_button.h"
#include "../panels/connection/connection_panel.h"
#include "../panels/telemetry/telemetry_panel.h"
#include "../panels/control/control_panel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QWindow>
#include <QTimer>
#include <QHBoxLayout>

TransparentWidget::TransparentWidget(QWidget *parent)
    : QWidget(parent)
{
    // Прозрачный фон
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
    raise();
    
    qDebug() << "[TransparentWidget] Created";
    
    // Таймер для определения длинного нажатия (0.2 секунды)
    m_pressTimer = new QTimer(this);
    m_pressTimer->setSingleShot(true);
    m_pressTimer->setInterval(200);
    
    connect(m_pressTimer, &QTimer::timeout, this, [this]() {
        m_timerExpired = true;
        qDebug() << "[TransparentWidget] Timer expired (200ms)";
    });
    
    // Создаём кнопки управления окном
    setupWindowButtons();
    
    // Создаём кнопки управления подвесом
    setupGimbalButtons();
    
    // Создаём панели
    setupPanels();
}

void TransparentWidget::setupWindowButtons()
{
    // Кнопка закрытия
    m_closeButton = new CloseWidget(this);
    m_closeButton->raise();
    connect(m_closeButton, &CloseWidget::clicked, this, [this]() {
        qDebug() << "[TransparentWidget] Close button clicked";
        emit closeClicked();
    });
    
    // Кнопка скрытия
    m_hideButton = new HideWidget(this);
    m_hideButton->raise();
    connect(m_hideButton, &HideWidget::clicked, this, [this]() {
        qDebug() << "[TransparentWidget] Hide button clicked";
        emit hideClicked();
    });
    
    // Кнопка изменения размера
    m_resizeButton = new ResizeWidget(this);
    m_resizeButton->raise();
    connect(m_resizeButton, &ResizeWidget::clicked, this, [this]() {
        qDebug() << "[TransparentWidget] Resize button clicked";
        emit resizeClicked();
    });
    
    // Позиционируем кнопки
    updateWindowButtonsPosition();
}

void TransparentWidget::updateWindowButtonsPosition()
{
    if (!m_closeButton || !m_hideButton || !m_resizeButton) {
        return;
    }

    int buttonWidth = 30;
    int buttonHeight = 30;
    int rightMargin = 0;
    int topMargin = 0;

    // Позиционируем кнопки справа налево в верхнем правом углу
    m_closeButton->setGeometry(width() - buttonWidth, topMargin, buttonWidth, buttonHeight);
    m_resizeButton->setGeometry(width() - buttonWidth * 2, topMargin, buttonWidth, buttonHeight);
    m_hideButton->setGeometry(width() - buttonWidth * 3, topMargin, buttonWidth, buttonHeight);
}

void TransparentWidget::setupGimbalButtons()
{
    // Кнопка подключения
    m_connectionButton = new ConnectionButton(this);
    m_connectionButton->raise();
    connect(m_connectionButton, &ConnectionButton::toggled, this, [this](bool active) {
        qDebug() << "[TransparentWidget] Connection button toggled:" << active;
        emit connectionToggled(active);
    });
    
    // Кнопка телеметрии
    m_telemetryButton = new TelemetryButton(this);
    m_telemetryButton->raise();
    connect(m_telemetryButton, &TelemetryButton::toggled, this, [this](bool active) {
        qDebug() << "[TransparentWidget] Telemetry button toggled:" << active;
        emit telemetryToggled(active);
    });
    
    // Кнопка управления
    m_controlButton = new ControlButton(this);
    m_controlButton->raise();
    connect(m_controlButton, &ControlButton::toggled, this, [this](bool active) {
        qDebug() << "[TransparentWidget] Control button toggled:" << active;
        emit controlToggled(active);
    });
    
    // Позиционируем кнопки
    updateGimbalButtonsPosition();
}

void TransparentWidget::updateGimbalButtonsPosition()
{
    if (!m_connectionButton || !m_telemetryButton || !m_controlButton) {
        return;
    }
    
    int buttonSize = 50;
    int rightMargin = 10;
    int spacing = 10;
    
    // Вычисляем общую высоту всех кнопок с отступами
    int totalHeight = buttonSize * 3 + spacing * 2;
    
    // Центрируем по вертикали
    int startY = (height() - totalHeight) / 2;
    int x = width() - buttonSize - rightMargin;
    
    m_connectionButton->setGeometry(x, startY, buttonSize, buttonSize);
    m_telemetryButton->setGeometry(x, startY + buttonSize + spacing, buttonSize, buttonSize);
    m_controlButton->setGeometry(x, startY + (buttonSize + spacing) * 2, buttonSize, buttonSize);
}

void TransparentWidget::setupPanels()
{
    // Панель подключения (справа сверху)
    m_connectionPanel = new ConnectionPanel(this);
    m_connectionPanel->raise();
    m_connectionPanel->setVisible(false);
    
    // Панель телеметрии (справа снизу)
    m_telemetryPanel = new TelemetryPanel(this);
    m_telemetryPanel->raise();
    m_telemetryPanel->setVisible(false);
    
    // Панель управления (слева снизу)
    m_controlPanel = new ControlPanel(this);
    m_controlPanel->raise();
    m_controlPanel->setVisible(false);
    
    // Позиционируем панели
    updatePanelsPosition();
    
    // Подключаем сигналы кнопок к видимости панелей
    connect(m_connectionButton, &ConnectionButton::toggled, this, [this](bool active) {
        m_connectionPanel->setVisible(active);
    });
    connect(m_telemetryButton, &TelemetryButton::toggled, this, [this](bool active) {
        m_telemetryPanel->setVisible(active);
    });
    connect(m_controlButton, &ControlButton::toggled, this, [this](bool active) {
        m_controlPanel->setVisible(active);
    });
}

void TransparentWidget::updatePanelsPosition()
{
    if (!m_connectionPanel || !m_telemetryPanel || !m_controlPanel) {
        return;
    }
    
    int connectionPanelWidth = 200;
    int connectionPanelHeight = 120;
    int margin = 10;
    
    // Панель подключения - справа сверху (под кнопками управления окном)
    m_connectionPanel->setGeometry(
        width() - connectionPanelWidth - margin,
        40,  // Отступ от кнопок управления окном
        connectionPanelWidth,
        connectionPanelHeight
    );

    int telemetryPanelWidth = 200;
    int telemetryPanelHeight = 150;
    
    // Панель телеметрии - справа снизу
    m_telemetryPanel->setGeometry(
        width() - telemetryPanelWidth - margin,
        height() - telemetryPanelHeight - margin,
        telemetryPanelWidth,
        telemetryPanelHeight
    );

    int controlPanelWidth = 200;
    int controlPanelHeight = 150;
    
    // Панель управления - слева снизу
    m_controlPanel->setGeometry(
        margin,
        height() - controlPanelHeight - margin,
        controlPanelWidth,
        controlPanelHeight
    );
}

void TransparentWidget::updatePanelsVisibility()
{
    if (m_connectionPanel && m_connectionButton) {
        m_connectionPanel->setVisible(m_connectionButton->isActive());
    }
    if (m_telemetryPanel && m_telemetryButton) {
        m_telemetryPanel->setVisible(m_telemetryButton->isActive());
    }
    if (m_controlPanel && m_controlButton) {
        m_controlPanel->setVisible(m_controlButton->isActive());
    }
}

void TransparentWidget::setFullscreen(bool fullscreen)
{
    m_fullscreen = fullscreen;
    qDebug() << "[TransparentWidget] setFullscreen:" << fullscreen;
}

bool TransparentWidget::event(QEvent* event)
{
    return QWidget::event(event);
}

void TransparentWidget::mousePressEvent(QMouseEvent* event)
{
    // Проверяем, находится ли нажатие в области кнопок
    QPoint pos = event->pos();
    
    // Игнорируем нажатия, если они были на кнопках управления окном
    if (m_closeButton && m_closeButton->geometry().contains(pos)) {
        return;
    }
    if (m_hideButton && m_hideButton->geometry().contains(pos)) {
        return;
    }
    if (m_resizeButton && m_resizeButton->geometry().contains(pos)) {
        return;
    }
    
    // Игнорируем нажатия, если они были на кнопках управления подвесом
    if (m_connectionButton && m_connectionButton->geometry().contains(pos)) {
        return;
    }
    if (m_telemetryButton && m_telemetryButton->geometry().contains(pos)) {
        return;
    }
    if (m_controlButton && m_controlButton->geometry().contains(pos)) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        // Сохраняем глобальную позицию мыши
        m_pressPosition = event->globalPosition().toPoint();
        
        // Получаем главное окно приложения
        QWidget* topLevel = nullptr;
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (widget->isActiveWindow()) {
                topLevel = widget;
                break;
            }
        }
        
        if (topLevel) {
            m_windowPos = topLevel->pos();
            m_topLevelWidget = topLevel;
            m_topLevelWindow = topLevel->windowHandle();
        }
        
        m_isDragging = false;
        m_timerExpired = false;
        m_mouseMoved = false;
        m_dragStarted = false;
        
        qDebug() << "[TransparentWidget] Mouse PRESS at" << m_pressPosition << "- timer started (200ms)";
        
        // Запускаем таймер
        m_pressTimer->start();
    }
    event->accept();
}

void TransparentWidget::mouseReleaseEvent(QMouseEvent* event)
{
    // Проверяем, находится ли курсор над кнопками - если да, игнорируем
    QPoint pos = event->pos();
    
    if (m_closeButton && m_closeButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_hideButton && m_hideButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_resizeButton && m_resizeButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_connectionButton && m_connectionButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_telemetryButton && m_telemetryButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_controlButton && m_controlButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        m_pressTimer->stop();
        
        qDebug() << "[TransparentWidget] Mouse RELEASE - timerExpired:" << m_timerExpired << "mouseMoved:" << m_mouseMoved;
        
        if (!m_timerExpired) {
            // 1.1 / 2.1) Короткое нажатие - захват цели
            qDebug() << "[TransparentWidget] -> SHORT PRESS (<200ms) - emitting targetAcquire()";
            emit targetAcquire();
        } else if (!m_mouseMoved) {
            // 1.2 / 2.2) Долгое нажатие без движения - отмена захвата
            qDebug() << "[TransparentWidget] -> LONG PRESS without movement (>=200ms, no move) - emitting targetCancel()";
            emit targetCancel();
        } else if (m_isDragging) {
            // Конец перетаскивания (режим окна)
            qDebug() << "[TransparentWidget] -> END DRAG - emitting endDrag()";
            emit endDrag();
            m_isDragging = false;
        }
        
        // Сбрасываем состояние
        m_dragStarted = false;
        m_mouseMoved = false;
        m_timerExpired = false;
        
        event->accept();
        return;
    }
}

void TransparentWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Проверяем, находится ли курсор над кнопками - если да, игнорируем движение
    QPoint pos = event->pos();
    
    if (m_closeButton && m_closeButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_hideButton && m_hideButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_resizeButton && m_resizeButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_connectionButton && m_connectionButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_telemetryButton && m_telemetryButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    if (m_controlButton && m_controlButton->geometry().contains(pos)) {
        event->ignore();
        return;
    }
    
    // Проверяем, что левая кнопка всё ещё зажата
    if (!(event->buttons() & Qt::LeftButton)) {
        if (m_dragStarted) {
            m_dragStarted = false;
            m_isDragging = false;
        }
        return;
    }
    
    // Если таймер ещё не истёк - ждём
    if (!m_timerExpired) {
        event->accept();
        return;
    }
    
    QPoint currentPos = event->globalPosition().toPoint();
    QPoint delta = currentPos - m_pressPosition;
    
    // Проверяем, что мышь сдвинулась
    if (!m_mouseMoved && (qAbs(delta.x()) > 1 || qAbs(delta.y()) > 1)) {
        m_mouseMoved = true;
        qDebug() << "[TransparentWidget] Mouse MOVED, delta:" << delta;
    }
    
    if (m_mouseMoved) {
        if (!m_fullscreen) {
            // 1.3) Режим окна - перетаскивание через startSystemMove
            if (!m_dragStarted) {
                m_isDragging = true;
                m_dragStarted = true;
                qDebug() << "[TransparentWidget] -> START DRAG (window mode) - emitting startDrag(), calling startSystemMove()";
                emit startDrag();
                
                // Вызываем startSystemMove - система берёт перетаскивание на себя
                if (m_topLevelWindow && !m_topLevelWidget->isMaximized()) {
                    m_topLevelWindow->startSystemMove();
                }
            }
            // После startSystemMove() перемещение обрабатывается системой
        } else {
            // 2.3) Полноэкранный режим - управление гимбалом
            qDebug() << "[TransparentWidget] -> GIMBAL MOVE (fullscreen mode) - emitting gimbalMove(" << delta << ")";
            emit gimbalMove(delta);
        }
    }
    
    event->accept();
}

void TransparentWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setBrush(Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

void TransparentWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Обновляем позицию кнопок и панелей при изменении размера
    updateWindowButtonsPosition();
    updateGimbalButtonsPosition();
    updatePanelsPosition();
}
