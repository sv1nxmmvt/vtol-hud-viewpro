#pragma once

#include <QWidget>

class TelemetryButton : public QWidget
{
    Q_OBJECT

public:
    explicit TelemetryButton(QWidget *parent = nullptr);
    ~TelemetryButton() override = default;

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
