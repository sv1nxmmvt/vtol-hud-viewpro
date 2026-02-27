#pragma once

#include <QWidget>

class CameraSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CameraSubPanel(QWidget *parent = nullptr);
    ~CameraSubPanel() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
};
