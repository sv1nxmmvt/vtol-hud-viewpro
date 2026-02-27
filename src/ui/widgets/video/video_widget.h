#pragma once

#include <QWidget>

class TransparentWidget;

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override = default;

    TransparentWidget* transparentWidget() const { return m_transparentWidget; }

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    TransparentWidget* m_transparentWidget = nullptr;
};
