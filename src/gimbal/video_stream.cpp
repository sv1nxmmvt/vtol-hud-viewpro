#include "video_stream.h"

#include <QCoreApplication>
#include <QDebug>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

namespace gimbal {

struct VideoStream::Impl {
    GstElement* pipeline = nullptr;
    GstElement* appsink = nullptr;
    std::string rtspUrl;
    int latency = 30;
    VideoStream* q = nullptr;
};

VideoStream::VideoStream(QObject *parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
    m_impl->q = this;
}

VideoStream::~VideoStream() {
    stop();
}

bool VideoStream::start(const std::string& rtspUrl, int latency) {
    if (m_impl->pipeline) {
        stop();
    }

    m_impl->rtspUrl = rtspUrl;
    m_impl->latency = latency;

    qDebug() << "VideoStream: creating pipeline with RTSP URL:" << QString::fromStdString(rtspUrl);

    // Создаём пайплайн
    // rtspsrc location=rtsp://... latency=30 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink sync=0
    std::string pipelineStr = 
        "rtspsrc location=" + rtspUrl + " latency=" + std::to_string(latency) + 
        " ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink name=sink sync=0 emit-signals=true";

    qDebug() << "VideoStream: pipeline:" << QString::fromStdString(pipelineStr);

    GError* error = nullptr;
    m_impl->pipeline = gst_parse_launch(pipelineStr.c_str(), &error);

    if (error) {
        QString errorMsg = QString("Failed to create pipeline: %1").arg(error->message);
        qCritical() << "VideoStream:" << errorMsg;
        g_error_free(error);
        emit errorOccurred(errorMsg);
        m_impl->pipeline = nullptr;
        return false;
    }

    qDebug() << "VideoStream: pipeline created successfully";

    // Получаем appsink
    m_impl->appsink = gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "sink");
    if (!m_impl->appsink) {
        qCritical() << "VideoStream: failed to get appsink element";
        emit errorOccurred("Failed to get appsink element");
        gst_object_unref(m_impl->pipeline);
        m_impl->pipeline = nullptr;
        return false;
    }

    // Настраиваем callback для получения кадров
    GstAppSinkCallbacks callbacks = {nullptr, nullptr, onNewSample};
    gst_app_sink_set_callbacks(GST_APP_SINK(m_impl->appsink), &callbacks, this, nullptr);

    // Настраиваем обработку сообщений шины
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_impl->pipeline));
    gst_bus_add_watch(bus, onBusMessage, this);
    gst_object_unref(bus);

    // Запускаем пайплайн
    qDebug() << "VideoStream: setting pipeline to PLAYING state";
    GstStateChangeReturn ret = gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "VideoStream: failed to start pipeline";
        emit errorOccurred("Failed to start pipeline");
        gst_object_unref(m_impl->appsink);
        m_impl->appsink = nullptr;
        gst_object_unref(m_impl->pipeline);
        m_impl->pipeline = nullptr;
        return false;
    }

    qDebug() << "VideoStream: pipeline started successfully";
    emit started();
    return true;
}

void VideoStream::stop() {
    if (m_impl->pipeline) {
        gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
        gst_object_unref(m_impl->appsink);
        m_impl->appsink = nullptr;
        gst_object_unref(m_impl->pipeline);
        m_impl->pipeline = nullptr;
        emit stopped();
    }
}

bool VideoStream::isPlaying() const {
    return m_impl->pipeline != nullptr;
}

GstFlowReturn VideoStream::onNewSample(GstAppSink* sink, gpointer userData) {
    auto* videoStream = static_cast<VideoStream*>(userData);

    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) {
        return GST_FLOW_OK;
    }

    GstCaps* caps = gst_sample_get_caps(sample);
    GstBuffer* buffer = gst_sample_get_buffer(sample);

    if (!caps || !buffer) {
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    // Получаем информацию о видео
    GstVideoInfo videoInfo;
    if (!gst_video_info_from_caps(&videoInfo, caps)) {
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    int width = GST_VIDEO_INFO_WIDTH(&videoInfo);
    int height = GST_VIDEO_INFO_HEIGHT(&videoInfo);

    // Мапим буфер для чтения данных
    GstMapInfo mapInfo;
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    // Создаём QImage из данных
    // GStreamer использует формат I420/YUV, videoconvert конвертирует в RGB
    QImage frame(mapInfo.data, width, height, GST_VIDEO_INFO_PLANE_STRIDE(&videoInfo, 0), 
                 QImage::Format_RGB888);
    
    // Копируем кадр (т.к. буфер будет освобождён)
    QImage frameCopy = frame.copy();

    gst_buffer_unmap(buffer, &mapInfo);
    gst_sample_unref(sample);

    qDebug() << "VideoStream: received frame" << width << "x" << height;

    // Отправляем кадр в основной поток
    QMetaObject::invokeMethod(videoStream, [videoStream, frameCopy]() {
        emit videoStream->frameReady(frameCopy);
    }, Qt::QueuedConnection);

    return GST_FLOW_OK;
}

gboolean VideoStream::onBusMessage(GstBus* bus, GstMessage* msg, gpointer userData) {
    auto* videoStream = static_cast<VideoStream*>(userData);

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* error;
            gchar* debug;
            gst_message_parse_error(msg, &error, &debug);
            QString errorMsg = QString("GStreamer error: %1").arg(error->message);
            g_error_free(error);
            g_free(debug);
            QMetaObject::invokeMethod(videoStream, [videoStream, errorMsg]() {
                emit videoStream->errorOccurred(errorMsg);
            }, Qt::QueuedConnection);
            break;
        }
        case GST_MESSAGE_WARNING: {
            GError* error;
            gchar* debug;
            gst_message_parse_warning(msg, &error, &debug);
            qWarning() << "GStreamer warning:" << error->message;
            g_error_free(error);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_EOS:
            qWarning() << "GStreamer: End of stream";
            break;
        case GST_MESSAGE_STATE_CHANGED:
            // Игнорируем сообщения о смене состояния
            break;
        default:
            break;
    }

    return TRUE;
}

} // namespace gimbal
