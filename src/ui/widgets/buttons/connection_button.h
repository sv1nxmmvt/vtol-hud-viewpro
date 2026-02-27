#pragma once

#include <QWidget>

class ConnectionButton : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionButton(QWidget *parent = nullptr);
    ~ConnectionButton() override = default;

    void setActive(bool active);
    bool isActive() const { return m_active; }

signals:
    void toggled(bool active);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool m_active = false;
    bool m_hovered = false;
};
