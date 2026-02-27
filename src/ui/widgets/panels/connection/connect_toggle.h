#pragma once

#include <QWidget>

class ConnectToggle : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectToggle(QWidget *parent = nullptr);
    ~ConnectToggle() override = default;

    void setState(bool connected);
    bool isConnected() const { return m_connected; }

signals:
    void toggled(bool connected);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_connected = false;
    bool m_hovered = false;
};
