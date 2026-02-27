#pragma once

#include <QWidget>

class ToggleSwitch;
class ZoomButton;

class LrfSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LrfSubPanel(QWidget *parent = nullptr);
    ~LrfSubPanel() override = default;

signals:
    void lrfModeChanged(bool isOnce);
    void lrfGetPressed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateControlsPosition();
    
    // Методы для отправки команд подвесу
    void sendLrfModeCommand(bool isOnce);
    void sendLrfGetCommand();

    ToggleSwitch* m_modeToggle = nullptr;
    ZoomButton* m_getButton = nullptr;
};
