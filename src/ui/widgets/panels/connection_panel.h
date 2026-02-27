#pragma once

#include <QWidget>

class ConnectionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPanel(QWidget *parent = nullptr);
    ~ConnectionPanel() override = default;

    void setVisible(bool visible) override;

protected:
    void paintEvent(QPaintEvent* event) override;
};
