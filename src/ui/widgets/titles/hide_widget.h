#pragma once

#include <QWidget>

class HideWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HideWidget(QWidget *parent = nullptr);
    ~HideWidget() override = default;

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
