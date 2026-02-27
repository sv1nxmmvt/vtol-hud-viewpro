#pragma once

#include <QLineEdit>

class PortInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit PortInput(QWidget *parent = nullptr);
    ~PortInput() override = default;

    int port() const { return text().toInt(); }
    void setPort(int port);
    void setEnabledBool(bool enabled);

private:
    bool m_enabled = true;
};
