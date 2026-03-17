#pragma once

#include <QWidget>

/**
 * @class DirectionButton
 * @brief Кнопка включения/выключения отображения направления (ориентации)
 *
 * Кнопка с иконкой компаса для отображения информации о ориентации
 * аппарата относительно севера (yaw/курс).
 */
class DirectionButton : public QWidget
{
    Q_OBJECT

public:
    explicit DirectionButton(QWidget *parent = nullptr);
    ~DirectionButton() override = default;

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
