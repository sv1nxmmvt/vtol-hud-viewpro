#pragma once

#include "gimbal_config.h"

#include <QObject>
#include <QImage>
#include <QMutex>
#include <memory>
#include <string>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

namespace gimbal {

// RTSP URL по умолчанию для Viewpro Q30TIRM Pro
constexpr const char* DEFAULT_RTSP_USER = "admin";
constexpr const char* DEFAULT_RTSP_PASSWORD = "159753";

/**
 * @class VideoStream
 * @brief Класс для получения и декодирования видеопотока через GStreamer
 *
 * Использует пайплайн: rtspsrc ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink
 */
class VideoStream : public QObject {
    Q_OBJECT

public:
    explicit VideoStream(QObject *parent = nullptr);
    ~VideoStream() override;

    /**
     * @brief Запустить видеопоток
     * @param config Конфигурация подключения
     * @param latency Задержка в мс (по умолчанию 30)
     * @return true при успехе
     */
    bool start(const ConnectionConfig& config, int latency = 30);

    /**
     * @brief Запустить видеопоток по RTSP URL
     * @param rtspUrl RTSP URL
     * @param latency Задержка в мс (по умолчанию 30)
     * @return true при успехе
     */
    bool start(const std::string& rtspUrl, int latency = 30);

    /**
     * @brief Остановить видеопоток
     */
    void stop();

    /**
     * @brief Проверить, запущен ли видеопоток
     */
    bool isPlaying() const;

    /**
     * @brief Построить RTSP URL из конфигурации
     * @param config Конфигурация подключения
     * @param user Пользователь (по умолчанию "admin")
     * @param password Пароль (по умолчанию "159753")
     * @return RTSP URL
     */
    static std::string buildRtspUrl(
        const ConnectionConfig& config,
        const std::string& user = DEFAULT_RTSP_USER,
        const std::string& password = DEFAULT_RTSP_PASSWORD
    );

signals:
    void frameReady(const QImage& frame);
    void errorOccurred(const QString& error);
    void started();
    void stopped();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    static GstFlowReturn onNewSample(GstAppSink* sink, gpointer userData);
    static gboolean onBusMessage(GstBus* bus, GstMessage* msg, gpointer userData);
};

} // namespace gimbal
