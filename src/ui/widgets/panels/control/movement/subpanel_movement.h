#pragma once

#include <QWidget>

class ToggleButton;
class ActionButton;
class ArrowButton;

class MovementSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MovementSubPanel(QWidget *parent = nullptr);
    ~MovementSubPanel() override = default;

signals:
    void motorsToggled(bool enabled);
    void homePressed();
    void directionPressed(const QString& direction);
    void directionReleased();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateControlsPosition();
    
    // Методы для отправки команд подвесу
    void sendMotorsCommand(bool enabled);
    void sendHomeCommand();
    void sendDirectionCommand(const QString& direction);

    ToggleButton* m_motorsButton = nullptr;
    ActionButton* m_homeButton = nullptr;
    
    ArrowButton* m_upButton = nullptr;
    ArrowButton* m_downButton = nullptr;
    ArrowButton* m_leftButton = nullptr;
    ArrowButton* m_rightButton = nullptr;
};
