#include "subpanel_camera.h"
#include "toggle_switch.h"
#include "zoom_button.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QMouseEvent>

CameraSubPanel::CameraSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    // Убираем WA_TransparentForMouseEvents, чтобы элементы управления могли получать события
    
    // Создаём тумблер Sony/TP
    m_sourceToggle = new ToggleSwitch(this, "SONY", "TP");
    connect(m_sourceToggle, &ToggleSwitch::toggled, this, [this](bool isSony) {
        m_isSony = isSony;
        qDebug() << "[CameraSubPanel] Source toggled to:" << (m_isSony ? "SONY" : "TP");
        emit sourceToggled(m_isSony);
        updateZoomButtons();
    });
    
    // Создаём тумблер PIP
    m_pipToggle = new ToggleSwitch(this, "PIP", "OFF");
    connect(m_pipToggle, &ToggleSwitch::toggled, this, [this](bool enabled) {
        qDebug() << "[CameraSubPanel] PIP toggled:" << (enabled ? "ON" : "OFF");
        emit pipToggled(enabled);
    });
    
    // Создаём кнопки зума
    m_zoomMinusButton = new ZoomButton(this, "-");
    connect(m_zoomMinusButton, &ZoomButton::pressed, this, [this]() {
        qDebug() << "[CameraSubPanel] Zoom - pressed (" << (m_isSony ? "OPT" : "DIG") << ")";
        emit zoomPressed(false, m_isSony);
    });
    connect(m_zoomMinusButton, &ZoomButton::released, this, [this]() {
        qDebug() << "[CameraSubPanel] Zoom - released";
        emit zoomReleased();
    });
    
    m_zoomPlusButton = new ZoomButton(this, "+");
    connect(m_zoomPlusButton, &ZoomButton::pressed, this, [this]() {
        qDebug() << "[CameraSubPanel] Zoom + pressed (" << (m_isSony ? "OPT" : "DIG") << ")";
        emit zoomPressed(true, m_isSony);
    });
    connect(m_zoomPlusButton, &ZoomButton::released, this, [this]() {
        qDebug() << "[CameraSubPanel] Zoom + released";
        emit zoomReleased();
    });
    
    updateZoomButtons();
}

void CameraSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(40, 40, 40, 60));
    
    // Рамка
    painter.setPen(QPen(QColor(255, 200, 100, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Заголовок
    painter.setPen(QColor(255, 200, 100));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(rect().adjusted(5, 5, -5, 25), Qt::AlignCenter, "CAMERA");
}

void CameraSubPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateControlsPosition();
}

void CameraSubPanel::updateControlsPosition()
{
    if (!m_sourceToggle || !m_pipToggle || !m_zoomPlusButton || !m_zoomMinusButton) {
        return;
    }
    
    int y = 35;
    int rowHeight = 30;
    int leftMargin = 10;
    
    // Тумблер Sony/TP
    m_sourceToggle->move(leftMargin, y);
    y += rowHeight;
    
    // Тумблер PIP
    m_pipToggle->move(leftMargin, y);
    y += rowHeight + 10;
    
    // Кнопки зума
    updateZoomButtons();
}

void CameraSubPanel::updateZoomButtons()
{
    if (!m_zoomPlusButton || !m_zoomMinusButton) {
        return;
    }
    
    int y = 35 + 30 + 30 + 15;
    int leftMargin = 10;
    
    // Метка
    QString zoomType = m_isSony ? "OPT" : "DIG";
    
    // Кнопка минус
    m_zoomMinusButton->move(leftMargin, y);
    
    // Кнопка плюс
    m_zoomPlusButton->move(leftMargin + 55, y);
}

void CameraSubPanel::mousePressEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}

void CameraSubPanel::mouseReleaseEvent(QMouseEvent* event)
{
    // Блокируем распространение события к родительским виджетам
    event->accept();
}
