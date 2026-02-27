#include "ip_address_input.h"
#include <QValidator>

IpAddressInput::IpAddressInput(QWidget *parent)
    : QLineEdit(parent)
{
    setFixedSize(130, 28);
    setText("192.168.2.119");
    setAlignment(Qt::AlignCenter);
    
    // Простой валидатор для IP-адреса
    setPlaceholderText("192.168.2.119");
    
    // Стили
    setStyleSheet(
        "QLineEdit {"
        "  background-color: rgba(40, 40, 60, 150);"
        "  border: 1px solid rgba(100, 100, 120, 200);"
        "  border-radius: 4px;"
        "  color: white;"
        "  padding: 2px;"
        "}"
        "QLineEdit:focus {"
        "  background-color: rgba(60, 60, 80, 200);"
        "  border: 1px solid rgba(100, 200, 255, 200);"
        "}"
        "QLineEdit:disabled {"
        "  background-color: rgba(30, 30, 40, 100);"
        "  border: 1px solid rgba(60, 60, 80, 200);"
        "  color: rgba(120, 120, 120, 200);"
        "}"
    );
}

void IpAddressInput::setIp(const QString& ip)
{
    setText(ip);
}

void IpAddressInput::setEnabledBool(bool enabled)
{
    m_enabled = enabled;
    setEnabled(enabled);
}
