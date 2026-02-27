#pragma once

#include <QWidget>

class MovementSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MovementSubPanel(QWidget *parent = nullptr);
    ~MovementSubPanel() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
};
