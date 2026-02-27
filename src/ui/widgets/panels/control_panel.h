#pragma once

#include <QWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel() override = default;

    void setVisible(bool visible) override;

protected:
    void paintEvent(QPaintEvent* event) override;
};
