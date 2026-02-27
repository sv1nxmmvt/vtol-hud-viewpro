#include "control_panel.h"
#include "camera/subpanel_camera.h"
#include "subpanel_lrf.h"
#include "subpanel_movement.h"
#include <QPainter>
#include <QResizeEvent>

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    // Убрали WA_TransparentForMouseEvents, чтобы кнопки могли получать события
    
    // Создаём подпанели как дочерние элементы
    m_cameraPanel = new CameraSubPanel(this);
    m_lrfPanel = new LrfSubPanel(this);
    m_movementPanel = new MovementSubPanel(this);
}

void ControlPanel::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible) {
        m_cameraPanel->show();
        m_lrfPanel->show();
        m_movementPanel->show();
    } else {
        m_cameraPanel->hide();
        m_lrfPanel->hide();
        m_movementPanel->hide();
    }
    update();
}

void ControlPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(30, 30, 30, 40));
    
    // Рамка панели
    painter.setPen(QPen(QColor(255, 200, 100, 200), 2));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void ControlPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSubPanelsPosition();
}

void ControlPanel::updateSubPanelsPosition()
{
    if (!m_cameraPanel || !m_lrfPanel || !m_movementPanel) {
        return;
    }
    
    int subPanelWidth = width() - 10;
    int subPanelHeight = (height() - 20) / 3;
    int spacing = 5;
    int margin = 5;
    
    int y = margin;
    
    // Camera подпанель (увеличенная высота)
    m_cameraPanel->setGeometry(margin, y, subPanelWidth, 140);
    y += 140 + spacing;
    
    // LRF подпанель
    m_lrfPanel->setGeometry(margin, y, subPanelWidth, subPanelHeight - spacing);
    y += subPanelHeight;
    
    // Movement подпанель
    m_movementPanel->setGeometry(margin, y, subPanelWidth, subPanelHeight);
}

void ControlPanel::mousePressEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}

void ControlPanel::mouseReleaseEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}
