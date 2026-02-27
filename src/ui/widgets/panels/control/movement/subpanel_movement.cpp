#include "subpanel_movement.h"
#include "toggle_button.h"
#include "action_button.h"
#include "arrow_button.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>

MovementSubPanel::MovementSubPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    
    // Кнопка MOTORS (тумблер)
    m_motorsButton = new ToggleButton(this, "MOTORS");
    connect(m_motorsButton, &ToggleButton::toggled, this, [this](bool enabled) {
        sendMotorsCommand(enabled);
    });
    
    // Кнопка HOME
    m_homeButton = new ActionButton(this, "HOME");
    connect(m_homeButton, &ActionButton::pressed, this, [this]() {
        sendHomeCommand();
    });
    
    // Кнопки со стрелочками
    m_upButton = new ArrowButton(this, "up");
    connect(m_upButton, &ArrowButton::pressed, this, [this]() {
        sendDirectionCommand("up");
    });
    connect(m_upButton, &ArrowButton::released, this, [this]() {
        emit directionReleased();
    });
    
    m_downButton = new ArrowButton(this, "down");
    connect(m_downButton, &ArrowButton::pressed, this, [this]() {
        sendDirectionCommand("down");
    });
    connect(m_downButton, &ArrowButton::released, this, [this]() {
        emit directionReleased();
    });
    
    m_leftButton = new ArrowButton(this, "left");
    connect(m_leftButton, &ArrowButton::pressed, this, [this]() {
        sendDirectionCommand("left");
    });
    connect(m_leftButton, &ArrowButton::released, this, [this]() {
        emit directionReleased();
    });
    
    m_rightButton = new ArrowButton(this, "right");
    connect(m_rightButton, &ArrowButton::pressed, this, [this]() {
        sendDirectionCommand("right");
    });
    connect(m_rightButton, &ArrowButton::released, this, [this]() {
        emit directionReleased();
    });
    
    updateControlsPosition();
}

void MovementSubPanel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Почти прозрачный фон
    painter.fillRect(rect(), QColor(40, 40, 40, 60));
    
    // Рамка
    painter.setPen(QPen(QColor(100, 200, 255, 150), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void MovementSubPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateControlsPosition();
}

void MovementSubPanel::updateControlsPosition()
{
    if (!m_motorsButton || !m_homeButton || 
        !m_upButton || !m_downButton || !m_leftButton || !m_rightButton) {
        return;
    }
    
    int leftMargin = 10;
    int topMargin = 5;
    int spacing = 5;
    
    // Левая колонка: MOTORS и HOME
    int leftColX = leftMargin;
    m_motorsButton->move(leftColX, topMargin);
    m_homeButton->move(leftColX, topMargin + m_motorsButton->height() + spacing - 1);  // Приподнята на 1px
    
    // Правая секция: стрелочки (таблица 2x2)
    int arrowSectionX = leftColX + m_motorsButton->width() + spacing * 2;
    int arrowSize = m_upButton->width();
    
    // Верхний ряд: ВВЕРХ (слева) / ВНИЗ (справа)
    m_upButton->move(arrowSectionX, topMargin);
    m_downButton->move(arrowSectionX + arrowSize + spacing, topMargin);
    
    // Нижний ряд: ВЛЕВО (слева) / ВПРАВО (справа) - приподняты на 3px
    int bottomRowY = topMargin + arrowSize + spacing - 3;
    m_leftButton->move(arrowSectionX, bottomRowY);
    m_rightButton->move(arrowSectionX + arrowSize + spacing, bottomRowY);
}

// Методы для отправки команд подвесу

void MovementSubPanel::sendMotorsCommand(bool enabled)
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->setMotorsEnabled(enabled);
    
    qDebug() << "[MovementSubPanel] sendMotorsCommand:" << (enabled ? "ON" : "OFF");
    emit motorsToggled(enabled);
}

void MovementSubPanel::sendHomeCommand()
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->goToHomePosition();
    
    qDebug() << "[MovementSubPanel] sendHomeCommand";
    emit homePressed();
}

void MovementSubPanel::sendDirectionCommand(const QString& direction)
{
    // TODO: Здесь будет логика отправки команды на подвес
    // Например: gimbal->moveDirection(direction);
    
    qDebug() << "[MovementSubPanel] sendDirectionCommand:" << direction;
    emit directionPressed(direction);
}
