#pragma once

#include <QLineEdit>

class IpAddressInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit IpAddressInput(QWidget *parent = nullptr);
    ~IpAddressInput() override = default;

    QString ip() const { return text(); }
    void setIp(const QString& ip);
    void setEnabledBool(bool enabled);

private:
    bool m_enabled = true;
};
