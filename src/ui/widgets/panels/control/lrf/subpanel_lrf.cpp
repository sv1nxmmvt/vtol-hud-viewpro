#include "subpanel_lrf.h"
#include "../camera/toggle_switch.h"
#include "../camera/zoom_button.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>

LrfSubPanel::LrfSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    
    // Создаём тумблер ONCE/LONG
    m_modeToggle = new ToggleSwitch(this, "ONCE", "LONG");
    connect(m_modeToggle, &ToggleSwitch::toggled, this, [this](bool isLong) {
        bool isOnce = !isLong;
        sendLrfModeCommand(isOnce);
        
        // Показываем/скрываем кнопку GET
        if (m_getButton) {
            m_getButton->setVisible(isOnce);
        }
    });
    
    // Создаём кнопку GET
    m_getButton = new ZoomButton(this, "GET");
    m_getButton->setFixedSize(50, 28);
    connect(m_getButton, &ZoomButton::pressed, this, [this]() {
        sendLrfGetCommand();
    });
    
    updateControlsPosition();
}

void LrfSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(40, 40, 40, 60));
    
    // Рамка
    painter.setPen(QPen(QColor(255, 100, 100, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Надпись LRF - слева (опущена на 3px)
    painter.setPen(QColor(255, 100, 100));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(10, 27, "LRF");
}

void LrfSubPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateControlsPosition();
}

void LrfSubPanel::updateControlsPosition()
{
    if (!m_modeToggle || !m_getButton) {
        return;
    }
    
    int y = 8;
    int leftMargin = 45;  // Отступ от надписи LRF
    
    // Тумблер ONCE/LONG
    m_modeToggle->move(leftMargin, y);
    
    // Кнопка GET - справа от тумблера
    m_getButton->move(leftMargin + m_modeToggle->width() + 5, y);
}

// Методы для отправки команд подвесу

void LrfSubPanel::sendLrfModeCommand(bool isOnce)
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->setLrfMode(isOnce ? LrfMode::Once : LrfMode::Long);
    
    qDebug() << "[LrfSubPanel] sendLrfModeCommand:" << (isOnce ? "ONCE" : "LONG");
    emit lrfModeChanged(isOnce);
}

void LrfSubPanel::sendLrfGetCommand()
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->lrfGet();
    
    qDebug() << "[LrfSubPanel] sendLrfGetCommand";
    emit lrfGetPressed();
}
