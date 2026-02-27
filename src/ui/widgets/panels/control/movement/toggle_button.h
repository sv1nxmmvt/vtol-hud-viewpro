#pragma once

#include <QWidget>

class ToggleButton : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleButton(QWidget *parent = nullptr, const QString& text = "MOTORS");
    ~ToggleButton() override = default;

    void setState(bool state);
    bool state() const { return m_state; }

signals:
    void toggled(bool state);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_state = false;
    bool m_hovered = false;
    QString m_text;
};
