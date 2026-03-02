#pragma once

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <memory>

class TransparentWidget;

/**
 * @class VideoWidget
 * @brief Виджет для отображения видеопотока с подвеса
 *
 * Отображает видео на QLabel, масштабируя его по размеру виджета.
 * Прозрачный виджет поверх видео обеспечивает элементы управления.
 */
class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override;

    TransparentWidget* transparentWidget() const { return m_transparentWidget; }

    /**
     * @brief Отобразить видеоролик
     * @param frame Кадр для отображения
     */
    void displayFrame(const QImage& frame);

    /**
     * @brief Очистить отображение видео
     */
    void clearDisplay();

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    TransparentWidget* m_transparentWidget = nullptr;
    QLabel* m_videoLabel = nullptr;
};
