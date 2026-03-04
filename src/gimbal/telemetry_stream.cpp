#include "telemetry_stream.h"

#include <QDebug>
#include <QMutexLocker>

namespace gimbal {

TelemetryStream* TelemetryStream::s_instance = nullptr;

TelemetryStream::TelemetryStream(QObject* parent)
    : QObject(parent)
{
    s_instance = this;
}

TelemetryStream::~TelemetryStream() {
    s_instance = nullptr;
}

void TelemetryStream::init(std::shared_ptr<Gimbal> gimbal) {
    if (!gimbal) {
        qWarning() << "[TelemetryStream] Invalid gimbal pointer";
        return;
    }

    m_gimbal = gimbal;

    // Регистрируем callback для получения телеметрии
    gimbal->setTelemetryCallback([this](const Telemetry& telemetry) {
        // Вызываем в главном потоке через мета-сигнал
        QMetaObject::invokeMethod(this, [this, telemetry]() {
            onTelemetryReceived(telemetry);
        }, Qt::QueuedConnection);
    });

    m_initialized = true;
    qDebug() << "[TelemetryStream] Initialized";
}

bool TelemetryStream::isInitialized() const {
    return m_initialized;
}

TelemetryStream& TelemetryStream::instance() {
    Q_ASSERT_X(s_instance, "TelemetryStream::instance()",
               "TelemetryStream not initialized. Call init() first.");
    return *s_instance;
}

Telemetry TelemetryStream::getTelemetry() const {
    return m_currentTelemetry;
}

void TelemetryStream::getAngles(double* yaw, double* pitch, double* roll) const {
    if (yaw) {
        *yaw = m_currentTelemetry.yaw;
    }
    if (pitch) {
        *pitch = m_currentTelemetry.pitch;
    }
    if (roll) {
        *roll = m_currentTelemetry.roll;
    }
}

double TelemetryStream::getZoom() const {
    return m_currentTelemetry.zoomMagTimes;
}

SensorType TelemetryStream::getSensorType() const {
    return m_currentTelemetry.sensorType;
}

TrackerStatus TelemetryStream::getTrackerStatus() const {
    return m_currentTelemetry.trackerStatus;
}

void TelemetryStream::getTargetCoordinates(double* lat, double* lng, double* alt) const {
    if (lat) {
        *lat = m_currentTelemetry.targetLat;
    }
    if (lng) {
        *lng = m_currentTelemetry.targetLng;
    }
    if (alt) {
        *alt = m_currentTelemetry.targetAlt;
    }
}

bool TelemetryStream::isTracking() const {
    return m_currentTelemetry.trackerStatus == TrackerStatus::Tracking;
}

void TelemetryStream::onTelemetryReceived(const Telemetry& telemetry) {
    {
        QMutexLocker locker(&m_mutex);
        m_currentTelemetry = telemetry;
    }

    // Отправляем сигналы вне мьютекса
    emit telemetryUpdated(telemetry);

    // Отладочное логирование телеметрии (раз в 100 обновлений)
    static int telemetryLogCounter = 0;
    if (++telemetryLogCounter % 100 == 0) {
        qDebug() << "[TelemetryStream] Telemetry:"
                 << "yaw=" << telemetry.yaw
                 << ", pitch=" << telemetry.pitch
                 << ", zoom=" << telemetry.zoomMagTimes
                 << ", sensor=" << static_cast<int>(telemetry.sensorType);
    }
}

} // namespace gimbal
