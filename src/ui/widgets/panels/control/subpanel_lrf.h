#pragma once

#include <QWidget>

class LrfSubPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LrfSubPanel(QWidget *parent = nullptr);
    ~LrfSubPanel() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
};
