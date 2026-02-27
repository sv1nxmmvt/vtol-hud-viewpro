#pragma once

#include <QWidget>

class ResizeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResizeWidget(QWidget *parent = nullptr);
    ~ResizeWidget() override = default;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool m_hovered = false;
};
