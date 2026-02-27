#include "transparent_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QWindow>
#include <QTimer>

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
}
