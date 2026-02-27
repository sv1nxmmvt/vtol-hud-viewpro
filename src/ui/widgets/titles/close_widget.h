#pragma once

#include <QWidget>

class CloseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CloseWidget(QWidget *parent = nullptr);
    ~CloseWidget() override = default;

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
