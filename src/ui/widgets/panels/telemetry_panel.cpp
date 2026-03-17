#include "telemetry_panel.h"
#include <QPainter>
#include <QFont>
#include <QResizeEvent>
#include <QDebug>

// Статические цвета
const QColor TelemetryPanel::TEXT_COLOR = QColor(144, 238, 144);  // Салатовый (lightgreen)
const QColor TelemetryPanel::BG_COLOR = QColor(30, 144, 255, TelemetryPanel::BG_ALPHA);  // Синий, как у кнопки телеметрии

TelemetryPanel::TelemetryPanel(QWidget *parent)
    : QWidget(parent)
{
    // Прозрачный фон по умолчанию
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);  // Чтобы не перехватывать клики
    
    // Скрываем панель по умолчанию
    hide();
    
    // Устанавливаем размер
    setFixedSize(260, 290);
    
    qDebug() << "[TelemetryPanel] Created";
}

void TelemetryPanel::updateTelemetry(const gimbal::MavlinkTelemetry& telemetry)
{
    m_telemetry = telemetry;
    update();  // Перерисовываем виджет
}

void TelemetryPanel::showPanel()
{
    m_visible = true;
    show();
    raise();  // Поднимаем на передний план
    qDebug() << "[TelemetryPanel] Shown";
}

void TelemetryPanel::hidePanel()
{
    m_visible = false;
    hide();
    qDebug() << "[TelemetryPanel] Hidden";
}

void TelemetryPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    if (!m_visible) {
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Рисуем фон
    drawBackground(painter);
    
    // Рисуем данные телеметрии
    drawTelemetryData(painter);
}

void TelemetryPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // При изменении размера перерисовываем
    update();
}

void TelemetryPanel::drawBackground(QPainter& painter)
{
    // Рисуем полупрозрачный прямоугольник с закругленными углами
    int radius = 10;
    painter.setBrush(BG_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), radius, radius);
    
    // Рисуем рамку
    painter.setPen(QPen(QColor(100, 180, 255, 150), 1));
    painter.drawRoundedRect(rect(), radius, radius);
}

void TelemetryPanel::drawTelemetryData(QPainter& painter)
{
    // Настройки шрифта
    QFont font = painter.font();
    font.setFamily("Consolas");  // Моноширинный шрифт для выравнивания
    font.setPointSize(9);
    painter.setFont(font);
    
    int leftMargin = 10;
    int topMargin = 10;
    int rowHeight = 20;
    int y = topMargin;
    
    // Заголовок
    QFont titleFont = font;
    titleFont.setPointSize(10);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(leftMargin, y + 15, "TELEMETRY");
    y += rowHeight + 5;
    
    // Разделительная линия
    painter.setFont(font);
    painter.setPen(QPen(QColor(100, 180, 255, 100), 1));
    painter.drawLine(leftMargin, y + 5, width() - leftMargin, y + 5);
    y += rowHeight - 5;
    
    // Координаты GPS
    QString gpsStr = formatCoordinates(m_telemetry.latitude_deg, m_telemetry.longitude_deg);
    drawParameterRow(painter, "GPS:", gpsStr, y + 10);
    y += rowHeight;

    // Высота
    QString altStr = formatAltitude(m_telemetry.relative_altitude_m);
    drawParameterRow(painter, "Alt:", altStr, y + 10);
    y += rowHeight;

    // Разделитель
    painter.setPen(QPen(QColor(100, 180, 255, 50), 1));
    painter.drawLine(leftMargin, y + 5, width() - leftMargin, y + 5);
    y += rowHeight - 8;

    // Ориентация (Roll, Pitch, Yaw)
    QString rollStr = formatAngle(m_telemetry.roll_deg);
    drawParameterRow(painter, "Roll:", rollStr, y + 10);
    y += rowHeight;

    QString pitchStr = formatAngle(m_telemetry.pitch_deg);
    drawParameterRow(painter, "Pitch:", pitchStr, y + 10);
    y += rowHeight;

    QString yawStr = formatAngle(m_telemetry.yaw_deg);
    drawParameterRow(painter, "Yaw:", yawStr, y + 10);
    y += rowHeight;

    // Разделитель
    painter.setPen(QPen(QColor(100, 180, 255, 50), 1));
    painter.drawLine(leftMargin, y + 5, width() - leftMargin, y + 5);
    y += rowHeight - 8;
    
    // Батарея
    QString batStr = formatBattery(m_telemetry.battery_percentage,
                                    m_telemetry.battery_voltage_v,
                                    m_telemetry.battery_current_a);
    drawParameterRow(painter, "Battery:", batStr, y + 10);
    y += rowHeight;

    // Режим полета
    drawParameterRow(painter, "Mode:", QString::fromStdString(m_telemetry.flight_mode), y + 10);
    y += rowHeight;

    // Вооружение
    QString armedStr = m_telemetry.armed ? "ARMED" : "DISARMED";
    QColor armedColor = m_telemetry.armed ? QColor(255, 100, 100) : QColor(100, 255, 100);
    painter.setPen(armedColor);
    drawParameterRow(painter, "Arming:", armedStr, y + 10);
    painter.setPen(TEXT_COLOR);  // Возвращаем цвет
    y += rowHeight;

    // Разделитель
    painter.setPen(QPen(QColor(100, 180, 255, 50), 1));
    painter.drawLine(leftMargin, y + 5, width() - leftMargin, y + 5);
    y += rowHeight - 8;

    // GPS статус
    QString gpsStatusStr = formatGpsStatus(m_telemetry.gps_fix_type,
                                            m_telemetry.gps_num_satellites,
                                            m_telemetry.gps_hdop);
    drawParameterRow(painter, "GPS Status:", gpsStatusStr, y + 10);
    y += rowHeight;

    // RC статус
    QString rcStatusStr = formatRcStatus(m_telemetry.rc_available, m_telemetry.rc_rssi_percent);
    drawParameterRow(painter, "RC:", rcStatusStr, y + 10);
    y += rowHeight;
}

void TelemetryPanel::drawParameterRow(QPainter& painter, const QString& label, const QString& value, int y)
{
    int leftMargin = 10;
    int labelWidth = 80;
    int rowHeight = 20;
    int fontOffset = 2;  // Смещение для базовой линии шрифта
    
    // Рисуем название параметра
    painter.setPen(QColor(180, 220, 255));  // Светло-голубой для названий
    painter.drawText(leftMargin, y + fontOffset, label);
    
    // Рисуем значение
    painter.setPen(TEXT_COLOR);  // Салатовый для значений
    painter.drawText(leftMargin + labelWidth, y + fontOffset - 13, width() - leftMargin * 2 - labelWidth, rowHeight, Qt::AlignRight, value);
}

QString TelemetryPanel::formatCoordinates(double lat, double lon) const
{
    return QString("%1°N %2°E")
        .arg(lat, 0, 'f', 6)
        .arg(lon, 0, 'f', 6);
}

QString TelemetryPanel::formatAltitude(double altitude) const
{
    return QString("%1 m").arg(altitude, 0, 'f', 1);
}

QString TelemetryPanel::formatSpeed(double speed) const
{
    return QString("%1 m/s").arg(speed, 0, 'f', 1);
}

QString TelemetryPanel::formatAngle(double angle) const
{
    return QString("%1°").arg(angle, 0, 'f', 1);
}

QString TelemetryPanel::formatBattery(float percent, float voltage, float current) const
{
    if (percent < 0.0f) {
        return QString("N/A  %1V  %2A")
            .arg(voltage, 0, 'f', 1)
            .arg(current, 0, 'f', 1);
    }
    return QString("%1%  %2V  %3A")
        .arg(percent * 100, 0, 'f', 0)
        .arg(voltage, 0, 'f', 1)
        .arg(current, 0, 'f', 1);
}

QString TelemetryPanel::formatGpsStatus(int fixType, int satellites, float hdop) const
{
    QString fixStr;
    switch (fixType) {
        case 0: fixStr = "No GPS"; break;
        case 1: fixStr = "No Fix"; break;
        case 2: fixStr = "2D Fix"; break;
        case 3: fixStr = "3D Fix"; break;
        case 4: fixStr = "DGPS"; break;
        case 5: fixStr = "RTK Float"; break;
        case 6: fixStr = "RTK Fixed"; break;
        default: fixStr = "Unknown"; break;
    }
    return QString("%1  (%2 sats, HDOP:%3)")
        .arg(fixStr)
        .arg(satellites)
        .arg(hdop, 0, 'f', 1);
}

QString TelemetryPanel::formatRcStatus(bool available, float rssi) const
{
    if (!available) {
        return "NO SIGNAL";
    }
    return QString("OK  (%1%)").arg(rssi, 0, 'f', 0);
}
