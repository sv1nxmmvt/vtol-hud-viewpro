#include "port_input.h"
#include <QValidator>

PortInput::PortInput(QWidget *parent)
    : QLineEdit(parent)
{
    setFixedSize(60, 28);
    setText("2000");
    setAlignment(Qt::AlignCenter);
    
    // Валидатор только для цифр (0-65535)
    QValidator* validator = new QIntValidator(0, 65535, this);
    setValidator(validator);
    
    setPlaceholderText("2000");
    
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

void PortInput::setPort(int port)
{
    setText(QString::number(port));
}

void PortInput::setEnabledBool(bool enabled)
{
    m_enabled = enabled;
    setEnabled(enabled);
}
