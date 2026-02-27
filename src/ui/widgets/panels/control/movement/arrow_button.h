#pragma once

#include <QWidget>

class ArrowButton : public QWidget
{
    Q_OBJECT

public:
    explicit ArrowButton(QWidget *parent = nullptr, const QString& direction = "up");
    ~ArrowButton() override = default;

    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return m_highlighted; }

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
    bool m_highlighted = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_direction;
};
