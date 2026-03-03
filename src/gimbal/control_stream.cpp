#include "control_stream.h"

#include <QThread>
#include <QElapsedTimer>
#include <QDebug>
#include <QtGlobal>
#include <QAtomicInteger>

namespace gimbal {

// ============================================================================
// Статические поля
// ============================================================================
QAtomicInteger<qint64> ControlStream::pitchSpeed{0};
QAtomicInteger<qint64> ControlStream::yawSpeed{0};
QAtomicInteger<int> ControlStream::zoomSpeed{0};
ControlStream* ControlStream::s_instance = nullptr;

// ============================================================================
// Конструктор / Деструктор
// ============================================================================
ControlStream::ControlStream(Gimbal& gimbal)
    : m_gimbal(gimbal)
{
    s_instance = this;
}

ControlStream::~ControlStream() {
    stop();
    s_instance = nullptr;
}

// ============================================================================
// Singleton access
// ============================================================================
ControlStream& ControlStream::instance(Gimbal& gimbal) {
    static ControlStream* instance = nullptr;
    if (!instance) {
        instance = new ControlStream(gimbal);
    }
    return *instance;
}

ControlStream& ControlStream::instance() {
    Q_ASSERT_X(s_instance, "ControlStream::instance()", 
               "ControlStream not initialized. Call instance(gimbal) first.");
    return *s_instance;
}

// ============================================================================
// Настройки частоты
// ============================================================================
void ControlStream::setFrequency(int frequency_hz) {
    if (frequency_hz < 1) {
        frequency_hz = 1;
    } else if (frequency_hz > 100) {
        frequency_hz = 100;
    }
    m_frequency_hz = frequency_hz;
}

int ControlStream::getFrequency() const {
    return m_frequency_hz;
}

// ============================================================================
// Управление потоком
// ============================================================================
void ControlStream::start() {
    if (!m_running) {
        m_running = 1;
        QThread::start();
    }
}

void ControlStream::stop() {
    if (m_running) {
        m_running = 0;
        wait(1000);  // Ждем завершения потока до 1 секунды
    }
}

bool ControlStream::isRunning() const {
    return m_running != 0;
}

void ControlStream::reset() {
    pitchSpeed = 0;
    yawSpeed = 0;
    zoomSpeed = 0;
}

// ============================================================================
// Поток отправки команд
// ============================================================================
void ControlStream::run() {
    // Период отправки в миллисекундах
    const int periodMs = 1000 / m_frequency_hz;
    QElapsedTimer timer;
    timer.start();

    qDebug() << "[ControlStream] started, frequency:" << m_frequency_hz << "Hz, period:" << periodMs << "ms";

    // Предыдущие значения для оптимизации (не отправлять одно и то же много раз)
    qint64 prevPitch = 0;
    qint64 prevYaw = 0;
    int prevZoom = 0;

    while (m_running) {
        // Читаем текущие значения управляющих сигналов
        qint64 pitch = pitchSpeed;
        qint64 yaw = yawSpeed;
        int zoom = zoomSpeed;

        // Отправляем команды на движение (если изменились)
        if (pitch != prevPitch || yaw != prevYaw) {
            if (pitch == 0 && yaw == 0) {
                m_gimbal.stop();
            } else {
                // Ограничиваем диапазон
                int16_t pitchClamped = static_cast<int16_t>(qBound(static_cast<qint64>(-2000), pitch, static_cast<qint64>(2000)));
                int16_t yawClamped = static_cast<int16_t>(qBound(static_cast<qint64>(-2000), yaw, static_cast<qint64>(2000)));
                m_gimbal.move(yawClamped, pitchClamped);
            }
            prevPitch = pitch;
            prevYaw = yaw;
        }

        // Отправляем команды на зум (если изменились)
        if (zoom != prevZoom) {
            if (zoom == 0) {
                m_gimbal.stopZoom();
            } else if (zoom > 0) {
                m_gimbal.zoomIn(static_cast<uint8_t>(zoom));
            } else {
                m_gimbal.zoomOut(static_cast<uint8_t>(-zoom));
            }
            prevZoom = zoom;
        }

        // Ждем следующего цикла
        qint64 elapsed = timer.elapsed();
        if (elapsed < periodMs) {
            QThread::msleep(static_cast<unsigned long>(periodMs - elapsed));
        }
        timer.restart();
    }

    // При остановке потока останавливаем и подвес
    m_gimbal.stop();
    m_gimbal.stopZoom();

    qDebug() << "[ControlStream] stopped";
}

} // namespace gimbal
