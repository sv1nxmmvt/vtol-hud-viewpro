#pragma once

#include <QWidget>

class TelemetryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TelemetryPanel(QWidget *parent = nullptr);
    ~TelemetryPanel() override = default;

    void setVisible(bool visible) override;

protected:
    void paintEvent(QPaintEvent* event) override;
};
