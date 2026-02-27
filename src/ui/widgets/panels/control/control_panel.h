#pragma once

#include <QWidget>

class CameraSubPanel;
class LrfSubPanel;
class MovementSubPanel;

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel() override = default;

    void setVisible(bool visible) override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void updateSubPanelsPosition();

    CameraSubPanel* m_cameraPanel = nullptr;
    LrfSubPanel* m_lrfPanel = nullptr;
    MovementSubPanel* m_movementPanel = nullptr;
};
