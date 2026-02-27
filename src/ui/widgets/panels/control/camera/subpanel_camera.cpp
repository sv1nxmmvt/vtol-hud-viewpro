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
    connect(m_sourceToggle, &ToggleSwitch::toggled, this, [this](bool isTP) {
        m_isSony = !isTP;  // Инвертируем: true = TP, false = SONY
        qDebug() << "[CameraSubPanel] Source toggled to:" << (m_isSony ? "SONY" : "TP");
        emit sourceToggled(m_isSony);
        update();  // Перерисовать для обновления метки OPT/DIG ZOOM
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
    
    // Метка OPT/DIG ZOOM - слева от кнопок
    QString zoomType = m_isSony ? "OPT ZOOM" : "DIG ZOOM";
    painter.setPen(QColor(200, 200, 200));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.drawText(10, 58, zoomType);
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

    int y = 5;  // Подняли тумблеры на место заголовка CAMERA
    int leftMargin = 10;
    int rightMargin = 10;

    // Тумблер Sony/TP - слева
    m_sourceToggle->move(leftMargin, y);

    // Тумблер PIP - справа
    m_pipToggle->move(width() - m_pipToggle->width() - rightMargin, y);

    // Кнопки зума - под тумблерами (минимальный отступ)
    updateZoomButtons();
}

void CameraSubPanel::updateZoomButtons()
{
    if (!m_zoomPlusButton || !m_zoomMinusButton) {
        return;
    }

    int y = 40;  // Отступ от тумблеров
    int labelWidth = 70;  // Ширина метки OPT/DIG ZOOM
    int leftMargin = 4;

    // Кнопка минус - справа от метки (сдвинута влево на 2px)
    m_zoomMinusButton->move(leftMargin + labelWidth + 3, y);

    // Кнопка плюс - справа от кнопки минус
    m_zoomPlusButton->move(leftMargin + labelWidth + 3 + m_zoomMinusButton->width() + 3, y);
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
