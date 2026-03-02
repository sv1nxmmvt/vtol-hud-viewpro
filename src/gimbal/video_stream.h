#pragma once

#include <QObject>
#include <QImage>
#include <QMutex>
#include <memory>
#include <string>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

namespace gimbal {

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

    bool start(const std::string& rtspUrl, int latency = 30);
    void stop();
    bool isPlaying() const;

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
