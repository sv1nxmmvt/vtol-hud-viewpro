#include "video_stream.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>

#include <sstream>
#include <cstring>

namespace gimbal {

struct VideoStream::Impl {
    GstElement* pipeline = nullptr;
    GstElement* appsink = nullptr;
    GstBus* bus = nullptr;
    GstVideoConverter* converter = nullptr;
    std::string rtspUrl;
    int latency = 30;
    VideoStream* q = nullptr;
    int frameCount = 0;
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

std::string VideoStream::buildRtspUrl(
    const ConnectionConfig& config,
    const std::string& user,
    const std::string& password
) {
    // Формат: rtsp://user:password@ip:videoPort/
    std::ostringstream oss;
    oss << "rtsp://" << user << ":" << password << "@"
        << config.ip << ":" << config.videoPort << "/";
    return oss.str();
}

bool VideoStream::start(const ConnectionConfig& config, int latency) {
    std::string rtspUrl = buildRtspUrl(config);
    qDebug() << "VideoStream: [CONFIG] type:" << QString::fromStdString(config.getTypeString())
             << ", ip:" << QString::fromStdString(config.ip)
             << ", port:" << config.port
             << ", videoPort:" << config.videoPort;
    qDebug() << "VideoStream: [RTSP] URL:" << QString::fromStdString(rtspUrl)
             << ", latency:" << latency;
    return start(rtspUrl, latency);
}

bool VideoStream::start(const std::string& rtspUrl, int latency) {
    qDebug() << "VideoStream: === STARTING VIDEO STREAM ===";
    
    if (m_impl->pipeline) {
        qDebug() << "VideoStream: stopping existing pipeline before starting new one";
        stop();
    }

    m_impl->rtspUrl = rtspUrl;
    m_impl->latency = latency;
    m_impl->frameCount = 0;

    // Инициализация GStreamer
    if (!gst_is_initialized()) {
        qDebug() << "VideoStream: initializing GStreamer";
        gst_init(nullptr, nullptr);
    }
    qDebug() << "VideoStream: GStreamer version:" << gst_version_string();

    // Создаём пайплайн с явной конвертацией в RGB
    // videoconvert ! video/x-raw,format=RGBx确保 appsink получает RGB данные
    std::string pipelineStr =
        "rtspsrc location=" + rtspUrl + " latency=" + std::to_string(latency) +
        " ! rtph264depay ! h264parse ! avdec_h264 ! "
        "videoconvert ! video/x-raw,format=RGBx ! "
        "appsink name=sink sync=0 emit-signals=true caps=video/x-raw,format=RGBx";

    qDebug() << "VideoStream: [PIPELINE] creating:" << QString::fromStdString(pipelineStr);

    GError* error = nullptr;
    m_impl->pipeline = gst_parse_launch(pipelineStr.c_str(), &error);

    if (error) {
        QString errorMsg = QString("Failed to create pipeline: %1").arg(error->message);
        qCritical() << "VideoStream: [ERROR] pipeline creation failed:" << errorMsg;
        g_error_free(error);
        emit errorOccurred(errorMsg);
        m_impl->pipeline = nullptr;
        return false;
    }

    if (!m_impl->pipeline) {
        qCritical() << "VideoStream: [ERROR] pipeline is NULL after gst_parse_launch";
        emit errorOccurred("Pipeline is NULL");
        return false;
    }

    qDebug() << "VideoStream: [PIPELINE] created successfully";

    // Получаем appsink
    m_impl->appsink = gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "sink");
    if (!m_impl->appsink) {
        qCritical() << "VideoStream: [ERROR] failed to get appsink element 'sink'";
        emit errorOccurred("Failed to get appsink element");
        gst_object_unref(m_impl->pipeline);
        m_impl->pipeline = nullptr;
        return false;
    }

    qDebug() << "VideoStream: [APPSINK] obtained successfully";

    // Настраиваем callback для получения кадров
    GstAppSinkCallbacks callbacks{};
    callbacks.new_sample = onNewSample;
    gst_app_sink_set_callbacks(GST_APP_SINK(m_impl->appsink), &callbacks, this, nullptr);

    // Включаем drop=false чтобы получать все кадры
    g_object_set(G_OBJECT(m_impl->appsink), "drop", FALSE, nullptr);
    qDebug() << "VideoStream: [APPSINK] callbacks set, drop=FALSE";

    // Настраиваем обработку сообщений шины
    m_impl->bus = gst_pipeline_get_bus(GST_PIPELINE(m_impl->pipeline));
    if (m_impl->bus) {
        gst_bus_add_watch(m_impl->bus, onBusMessage, this);
        gst_object_unref(m_impl->bus);
        qDebug() << "VideoStream: [BUS] message watch added";
    } else {
        qWarning() << "VideoStream: [WARNING] failed to get bus";
    }

    // Запускаем пайплайн
    qDebug() << "VideoStream: [STATE] setting pipeline to PLAYING...";
    GstStateChangeReturn ret = gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
    
    switch (ret) {
        case GST_STATE_CHANGE_SUCCESS:
            qDebug() << "VideoStream: [STATE] -> PLAYING (SUCCESS)";
            break;
        case GST_STATE_CHANGE_ASYNC:
            qDebug() << "VideoStream: [STATE] -> PLAYING (ASYNC - will complete later)";
            break;
        case GST_STATE_CHANGE_NO_PREROLL:
            qDebug() << "VideoStream: [STATE] -> PLAYING (NO_PREROLL - live source)";
            break;
        case GST_STATE_CHANGE_FAILURE:
            qCritical() << "VideoStream: [STATE] FAILED to go to PLAYING";
            emit errorOccurred("Failed to start pipeline");
            gst_object_unref(m_impl->appsink);
            m_impl->appsink = nullptr;
            gst_object_unref(m_impl->pipeline);
            m_impl->pipeline = nullptr;
            return false;
    }

    // Планируем проверку состояния через 2 секунды
    QTimer::singleShot(2000, this, [this]() {
        if (m_impl->pipeline) {
            GstState state;
            GstState pending;
            gst_element_get_state(m_impl->pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
            qDebug() << "VideoStream: [STATE CHECK] current:" << gst_element_state_get_name(state)
                     << ", pending:" << gst_element_state_get_name(pending);
            
            if (state != GST_STATE_PLAYING && state != GST_STATE_PAUSED) {
                qWarning() << "VideoStream: [WARNING] pipeline not in PLAYING state after 2s";
            }
        }
    });

    qDebug() << "VideoStream: === VIDEO STREAM STARTED ===";
    emit started();
    return true;
}

void VideoStream::stop() {
    qDebug() << "VideoStream: === STOPPING VIDEO STREAM ===";
    
    if (m_impl->pipeline) {
        qDebug() << "VideoStream: [STATE] setting pipeline to NULL...";
        GstStateChangeReturn ret = gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
        qDebug() << "VideoStream: [STATE] result:" << ret;
        
        gst_object_unref(m_impl->appsink);
        m_impl->appsink = nullptr;
        gst_object_unref(m_impl->pipeline);
        m_impl->pipeline = nullptr;
        m_impl->bus = nullptr;
        
        qDebug() << "VideoStream: pipeline resources released";
    }
    
    qDebug() << "VideoStream: frame count received:" << m_impl->frameCount;
    qDebug() << "VideoStream: === VIDEO STREAM STOPPED ===";
    emit stopped();
}

bool VideoStream::isPlaying() const {
    if (!m_impl->pipeline) {
        return false;
    }
    GstState state;
    gst_element_get_state(m_impl->pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    return state >= GST_STATE_PAUSED;
}

GstFlowReturn VideoStream::onNewSample(GstAppSink* sink, gpointer userData) {
    auto* videoStream = static_cast<VideoStream*>(userData);
    videoStream->m_impl->frameCount++;

    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) {
        if (videoStream->m_impl->frameCount % 30 == 1) {
            qDebug() << "VideoStream: [FRAME] pull_sample returned NULL (frame" << videoStream->m_impl->frameCount << ")";
        }
        return GST_FLOW_OK;
    }

    GstCaps* caps = gst_sample_get_caps(sample);
    GstBuffer* buffer = gst_sample_get_buffer(sample);

    if (!caps) {
        qWarning() << "VideoStream: [FRAME] caps is NULL";
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
    
    if (!buffer) {
        qWarning() << "VideoStream: [FRAME] buffer is NULL";
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    // Получаем информацию о видео
    GstVideoInfo videoInfo;
    if (!gst_video_info_from_caps(&videoInfo, caps)) {
        qWarning() << "VideoStream: [FRAME] failed to get video info from caps";
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    int width = GST_VIDEO_INFO_WIDTH(&videoInfo);
    int height = GST_VIDEO_INFO_HEIGHT(&videoInfo);
    GstVideoFormat format = GST_VIDEO_INFO_FORMAT(&videoInfo);
    
    if (videoStream->m_impl->frameCount <= 5 || videoStream->m_impl->frameCount % 30 == 1) {
        qDebug() << "VideoStream: [FRAME #" << videoStream->m_impl->frameCount << "]"
                 << "size:" << width << "x" << height
                 << ", format:" << gst_video_format_to_string(format);
    }

    // Мапим буфер для чтения данных
    GstMapInfo mapInfo;
    if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
        qWarning() << "VideoStream: [FRAME] failed to map buffer";
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    // Для RGBx формата (4 байта на пиксель)
    int stride = GST_VIDEO_INFO_PLANE_STRIDE(&videoInfo, 0);
    
    if (videoStream->m_impl->frameCount <= 3) {
        qDebug() << "VideoStream: [FRAME] buffer size:" << mapInfo.size
                 << ", stride:" << stride
                 << ", expected stride:" << (width * 4);
    }

    // Создаём QImage из RGBx данных
    // RGBx = 4 байта на пиксель, Format_RGB32 подходит
    QImage frame(mapInfo.data, width, height, stride, QImage::Format_RGB32);
    
    // Копируем кадр (т.к. буфер будет освобождён)
    QImage frameCopy = frame.copy();

    gst_buffer_unmap(buffer, &mapInfo);
    gst_sample_unref(sample);

    if (videoStream->m_impl->frameCount <= 5 || videoStream->m_impl->frameCount % 30 == 1) {
        qDebug() << "VideoStream: [FRAME] copied:" << frameCopy.size()
                 << ", depth:" << frameCopy.depth()
                 << ", bytesPerLine:" << frameCopy.bytesPerLine()
                 << ", isNull:" << frameCopy.isNull()
                 << ", format:" << frameCopy.format();
    }

    if (frameCopy.isNull()) {
        qWarning() << "VideoStream: [FRAME] frame copy is null after conversion";
        return GST_FLOW_OK;
    }

    // Отправляем кадр в основной поток
    QMetaObject::invokeMethod(videoStream, [videoStream, frameCopy, count = videoStream->m_impl->frameCount]() {
        emit videoStream->frameReady(frameCopy);
        if (count <= 5 || count % 30 == 1) {
            qDebug() << "VideoStream: [EMIT] frameReady signal emitted (frame" << count << ")";
        }
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
            QString errorMsg = QString("GStreamer error: %1 (debug: %2)").arg(error->message).arg(debug ? debug : "none");
            qCritical() << "VideoStream: [BUS ERROR]" << errorMsg;
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
            qWarning() << "VideoStream: [BUS WARNING]:" << error->message 
                       << (debug ? QString(" (debug: %1)").arg(debug) : "");
            g_error_free(error);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_EOS:
            qWarning() << "VideoStream: [BUS] End of stream";
            break;
        case GST_MESSAGE_STATE_CHANGED: {
            GstState oldState, newState, pending;
            gst_message_parse_state_changed(msg, &oldState, &newState, &pending);
            // Логируем только изменения состояния пайплайна (не элементов)
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(videoStream->m_impl->pipeline)) {
                qDebug() << "VideoStream: [STATE CHANGE]" 
                         << gst_element_state_get_name(oldState) << "->" 
                         << gst_element_state_get_name(newState)
                         << "(pending:" << gst_element_state_get_name(pending) << ")";
            }
            break;
        }
        case GST_MESSAGE_STREAM_STATUS: {
            GstStreamStatusType status;
            GstElement* owner;
            gst_message_parse_stream_status(msg, &status, &owner);
            const char* statusStr = "";
            switch (status) {
                case GST_STREAM_STATUS_TYPE_CREATE: statusStr = "CREATE"; break;
                case GST_STREAM_STATUS_TYPE_ENTER: statusStr = "ENTER"; break;
                case GST_STREAM_STATUS_TYPE_LEAVE: statusStr = "LEAVE"; break;
                case GST_STREAM_STATUS_TYPE_DESTROY: statusStr = "DESTROY"; break;
                default: statusStr = "UNKNOWN"; break;
            }
            qDebug() << "VideoStream: [STREAM STATUS]" << statusStr;
            break;
        }
        case GST_MESSAGE_ELEMENT: {
            // Проверяем на сообщения о подготовке потока
            const GstStructure* s = gst_message_get_structure(msg);
            if (s && gst_structure_has_name(s, "GstRTSPSource")) {
                qDebug() << "VideoStream: [RTSP SOURCE] element message received";
            }
            break;
        }
        default:
            break;
    }

    return TRUE;
}

} // namespace gimbal
