#pragma once

#include <QWidget>

class ToggleSwitch : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleSwitch(QWidget *parent = nullptr, const QString& textLeft = "A", const QString& textRight = "B");
    ~ToggleSwitch() override = default;

    void setState(bool state);
    bool state() const { return m_state; }

signals:
    void toggled(bool state);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_state = false;
    QString m_textLeft;
    QString m_textRight;
};
