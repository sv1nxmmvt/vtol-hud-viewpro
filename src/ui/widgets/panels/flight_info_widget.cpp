#include "flight_info_widget.h"
#include <QPainter>
#include <QFont>
#include <QResizeEvent>
#include <QDebug>

// Статические цвета
const QColor FlightInfoWidget::TEXT_COLOR = QColor(144, 238, 144);  // Салатовый (lightgreen)
const QColor FlightInfoWidget::BG_COLOR = QColor(0, 0, 0, 0);  // Полностью прозрачный

FlightInfoWidget::FlightInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    // Прозрачный фон по умолчанию
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);  // Чтобы не перехватывать клики

    // Устанавливаем размер
    setFixedSize(220, 210);

    qDebug() << "[FlightInfoWidget] Created";
}

void FlightInfoWidget::updateTelemetry(const gimbal::MavlinkTelemetry& telemetry)
{
    m_telemetry = telemetry;
    update();  // Перерисовываем виджет
}

void FlightInfoWidget::showPanel()
{
    m_visible = true;
    show();
    raise();
    qDebug() << "[FlightInfoWidget] Shown";
}

void FlightInfoWidget::hidePanel()
{
    m_visible = false;
    hide();
    qDebug() << "[FlightInfoWidget] Hidden";
}

void FlightInfoWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!m_visible) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Рисуем данные (без фона)
    drawFlightData(painter);
}

void FlightInfoWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void FlightInfoWidget::drawBackground(QPainter& painter)
{
    Q_UNUSED(painter);
    // Фон не рисуется - полностью прозрачный
}

void FlightInfoWidget::drawFlightData(QPainter& painter)
{
    // Настройки шрифта - уменьшенный на 20% (14 вместо 18)
    QFont font = painter.font();
    font.setFamily("Consolas");  // Моноширинный шрифт для выравнивания
    font.setPointSize(14);
    painter.setFont(font);

    int leftMargin = 8;   // Уменьшенный отступ на 20%
    int topMargin = 8;    // Уменьшенный отступ на 20%
    int rowHeight = 28;   // Уменьшенный интервал на 20%
    int y = topMargin + 12;

    // Высота абсолютная
    int absAlt = static_cast<int>(m_telemetry.absolute_altitude_m);
    QString absAltStr = formatAltitude(absAlt);
    drawParameterRow(painter, "Alt ABS:", absAltStr, y);
    y += rowHeight;

    // Высота относительная
    int relAlt = static_cast<int>(m_telemetry.relative_altitude_m);
    QString relAltStr = formatAltitude(relAlt);
    drawParameterRow(painter, "Alt REL:", relAltStr, y);
    y += rowHeight;

    // Вертикальная скорость
    int vertSpeed = static_cast<int>(m_telemetry.vertical_speed_m_s);
    QString vertSpeedStr = formatVerticalSpeed(vertSpeed);
    drawParameterRow(painter, "V.Speed:", vertSpeedStr, y);
    y += rowHeight;

    // Горизонтальная скорость
    int speed = static_cast<int>(m_telemetry.speed_m_s);
    QString speedStr = formatSpeed(speed);
    drawParameterRow(painter, "Speed:", speedStr, y);
    y += rowHeight;

    // Расстояние до дома
    int distance = static_cast<int>(m_telemetry.distance_to_home_m);
    QString distanceStr = formatDistance(distance);
    drawParameterRow(painter, "To Home:", distanceStr, y);
}

void FlightInfoWidget::drawParameterRow(QPainter& painter, const QString& label, const QString& value, int y)
{
    painter.setPen(TEXT_COLOR);
    painter.drawText(10, y, label);

    // Значение справа
    QFontMetrics fm(painter.font());
    int valueWidth = fm.horizontalAdvance(value);
    int rightMargin = 10;
    painter.drawText(width() - valueWidth - rightMargin, y, value);
}

QString FlightInfoWidget::formatAltitude(int altitude) const
{
    return QString("%1 m").arg(altitude);
}

QString FlightInfoWidget::formatVerticalSpeed(int speed) const
{
    return QString("%1 m/s").arg(speed);
}

QString FlightInfoWidget::formatSpeed(int speed) const
{
    return QString("%1 m/s").arg(speed);
}

QString FlightInfoWidget::formatDistance(int distance) const
{
    return QString("%1 m").arg(distance);
}
