#pragma once

#include <QWidget>

class ToggleSwitch;
class ZoomButton;

class CameraSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CameraSubPanel(QWidget *parent = nullptr);
    ~CameraSubPanel() override = default;

signals:
    void sourceToggled(bool isSony);
    void pipToggled(bool enabled);
    void zoomPressed(bool isPlus, bool isOptical);
    void zoomReleased();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void updateControlsPosition();
    void updateZoomButtons();

    ToggleSwitch* m_sourceToggle = nullptr;
    ToggleSwitch* m_pipToggle = nullptr;
    ZoomButton* m_zoomPlusButton = nullptr;
    ZoomButton* m_zoomMinusButton = nullptr;
    
    bool m_isSony = true;
};
