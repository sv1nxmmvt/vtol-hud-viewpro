#pragma once

#include <QWidget>

class IpAddressInput;
class PortInput;
class ConnectToggle;

class ConnectionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPanel(QWidget *parent = nullptr);
    ~ConnectionPanel() override = default;

    void setVisible(bool visible) override;

signals:
    void connectToggled(bool connected);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void updateControlsPosition();
    
    // Методы для отправки команд подвесу
    void sendConnectCommand(bool connected);

    IpAddressInput* m_ipInput = nullptr;
    PortInput* m_portInput = nullptr;
    ConnectToggle* m_connectToggle = nullptr;
};
