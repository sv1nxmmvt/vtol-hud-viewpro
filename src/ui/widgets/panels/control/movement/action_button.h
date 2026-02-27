#pragma once

#include <QWidget>

class ActionButton : public QWidget
{
    Q_OBJECT

public:
    explicit ActionButton(QWidget *parent = nullptr, const QString& text = "HOME");
    ~ActionButton() override = default;

    void setPressed(bool pressed);
    bool isPressed() const { return m_pressed; }

signals:
    void pressed();
    void released();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool m_pressed = false;
    bool m_hovered = false;
    QString m_text;
};
