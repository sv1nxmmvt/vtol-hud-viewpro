#include "direction_panel.h"
#include <QPainter>
#include <QFont>
#include <QResizeEvent>
#include <QDebug>
#include <cmath>

// Статические цвета
const QColor DirectionPanel::TEXT_COLOR = QColor(144, 238, 144);  // Салатовый
const QColor DirectionPanel::BG_COLOR = QColor(50, 205, 50, DirectionPanel::BG_ALPHA);  // Зелёный
const QColor DirectionPanel::INDICATOR_COLOR = QColor(150, 255, 150);  // Светло-зелёный для индикаторов

DirectionPanel::DirectionPanel(QWidget *parent)
    : QWidget(parent)
{
    // Прозрачный фон по умолчанию
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // Скрываем панель по умолчанию
    hide();

    // Устанавливаем размер (горизонтальное расположение)
    setFixedSize(280, 110);

    qDebug() << "[DirectionPanel] Created";
}

void DirectionPanel::updateTelemetry(const gimbal::MavlinkTelemetry& telemetry)
{
    m_telemetry = telemetry;
    update();  // Перерисовываем виджет
}

void DirectionPanel::showPanel()
{
    m_visible = true;
    show();
    raise();
    qDebug() << "[DirectionPanel] Shown";
}

void DirectionPanel::hidePanel()
{
    m_visible = false;
    hide();
    qDebug() << "[DirectionPanel] Hidden";
}

void DirectionPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!m_visible) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);
    drawIndicators(painter);
}

void DirectionPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void DirectionPanel::drawBackground(QPainter& painter)
{
    int radius = 10;
    painter.setBrush(BG_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), radius, radius);

    painter.setPen(QPen(QColor(100, 255, 100, 150), 1));
    painter.drawRoundedRect(rect(), radius, radius);
}

void DirectionPanel::drawIndicators(QPainter& painter)
{
    int topMargin = 5;
    int indicatorSize = 70;
    int spacing = 10;

    // Вычисляем общую ширину для центрирования
    int totalWidth = indicatorSize * 3 + spacing * 2;
    int startX = (width() - totalWidth) / 2;
    int y = topMargin;

    // Компас (направление/yaw)
    drawCompassIndicator(painter, startX, y, indicatorSize);

    // Тангаж (pitch)
    drawPitchIndicator(painter, startX + indicatorSize + spacing, y, indicatorSize);

    // Крен (roll)
    drawRollIndicator(painter, startX + (indicatorSize + spacing) * 2, y, indicatorSize);
}

void DirectionPanel::drawCompassIndicator(QPainter& painter, int x, int y, int size)
{
    int cx = x + size / 2;
    int cy = y + size / 2;
    int radius = size / 2 - 2;

    // Круг
    painter.setPen(QPen(INDICATOR_COLOR, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Линия от центра к направлению (yaw)
    // 0° = север (вверх), угол растёт по часовой стрелке
    double yawRad = (m_telemetry.yaw_deg - 90.0) * M_PI / 180.0;
    int lineEndX = cx + static_cast<int>(radius * std::cos(yawRad));
    int lineEndY = cy + static_cast<int>(radius * std::sin(yawRad));

    painter.setPen(QPen(INDICATOR_COLOR, 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(cx, cy, lineEndX, lineEndY);

    // Точка в центре
    painter.setBrush(INDICATOR_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 3, cy - 3, 6, 6);

    // Маркеры сторон света (N, E, S, W) - еле заметные
    QFont font = painter.font();
    font.setFamily("Arial");
    font.setPointSize(7);
    painter.setFont(font);
    painter.setPen(QColor(100, 180, 100, 150));  // Тусклый зелёный
    
    // N (север) - сверху
    painter.drawText(cx - 4, cy - radius + 8, 8, 10, Qt::AlignCenter, "N");
    // S (юг) - снизу
    painter.drawText(cx - 4, cy + radius - 16, 8, 10, Qt::AlignCenter, "S");
    // E (восток) - справа
    painter.drawText(cx + radius - 14, cy - 6, 10, 12, Qt::AlignCenter, "E");
    // W (запад) - слева
    painter.drawText(cx - radius + 4, cy - 6, 10, 12, Qt::AlignCenter, "W");

    // Текст с направлением под индикатором
    QString dirStr = formatDirection(m_telemetry.yaw_deg);
    drawIndicatorLabel(painter, "Dir", dirStr, x, y + size);
}

void DirectionPanel::drawPitchIndicator(QPainter& painter, int x, int y, int size)
{
    int cx = x + size / 2;
    int cy = y + size / 2;
    int radius = size / 2 - 2;

    // Круг
    painter.setPen(QPen(INDICATOR_COLOR, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Пунктирная линия нулевого положения (горизонтально - 0° pitch = вправо)
    painter.setPen(QPen(QColor(100, 180, 100, 100), 1, Qt::DashLine));
    painter.drawLine(cx - radius, cy, cx + radius, cy);

    // Линия от центра (pitch)
    // 0° = вправо, +90° (нос вверх) = вверх, -90° (нос вниз) = вниз
    double pitchRad = -m_telemetry.pitch_deg * M_PI / 180.0;
    int lineEndX = cx + static_cast<int>(radius * std::cos(pitchRad));
    int lineEndY = cy + static_cast<int>(radius * std::sin(pitchRad));

    painter.setPen(QPen(INDICATOR_COLOR, 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(cx, cy, lineEndX, lineEndY);

    // Точка в центре
    painter.setBrush(INDICATOR_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 3, cy - 3, 6, 6);

    // Текст с тангажом под индикатором
    QString pitchStr = formatAngle(m_telemetry.pitch_deg);
    drawIndicatorLabel(painter, "Pitch", pitchStr, x, y + size);
}

void DirectionPanel::drawRollIndicator(QPainter& painter, int x, int y, int size)
{
    int cx = x + size / 2;
    int cy = y + size / 2;
    int radius = size / 2 - 2;

    // Круг
    painter.setPen(QPen(INDICATOR_COLOR, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Пунктирная линия нулевого положения (горизонтально - 0° roll)
    painter.setPen(QPen(QColor(100, 180, 100, 100), 1, Qt::DashLine));
    painter.drawLine(cx - radius, cy, cx + radius, cy);

    // Диаметр (линия через центр) для крена
    // 0° = горизонтально, + = наклон вправо, - = наклон влево
    double rollRad = -m_telemetry.roll_deg * M_PI / 180.0;
    int lineEnd1X = cx + static_cast<int>(radius * std::cos(rollRad));
    int lineEnd1Y = cy + static_cast<int>(radius * std::sin(rollRad));
    int lineEnd2X = cx - static_cast<int>(radius * std::cos(rollRad));
    int lineEnd2Y = cy - static_cast<int>(radius * std::sin(rollRad));

    painter.setPen(QPen(INDICATOR_COLOR, 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(lineEnd1X, lineEnd1Y, lineEnd2X, lineEnd2Y);

    // Точка в центре
    painter.setBrush(INDICATOR_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 3, cy - 3, 6, 6);

    // Текст с креном под индикатором
    QString rollStr = formatAngle(m_telemetry.roll_deg);
    drawIndicatorLabel(painter, "Roll", rollStr, x, y + size);
}

void DirectionPanel::drawIndicatorLabel(QPainter& painter, const QString& label, const QString& value, int x, int y)
{
    QFont font = painter.font();
    font.setFamily("Consolas");
    font.setPointSize(8);
    painter.setFont(font);

    int indicatorWidth = 70;
    int textY = y + 10;  // Приподняли (было +18)
    int centerX = x + indicatorWidth / 2;

    // Название (над значением)
    painter.setPen(QColor(180, 255, 180));
    painter.drawText(centerX - 20, textY, 40, 12, Qt::AlignCenter, label);

    // Значение (под названием)
    painter.setPen(TEXT_COLOR);
    painter.drawText(centerX - 20, textY + 10, 40, 12, Qt::AlignCenter, value);
}

QString DirectionPanel::formatDirection(double yaw) const
{
    return QString("%1°").arg(yaw, 0, 'f', 1);
}

QString DirectionPanel::formatAngle(double angle) const
{
    return QString("%1°").arg(angle, 0, 'f', 1);
}
