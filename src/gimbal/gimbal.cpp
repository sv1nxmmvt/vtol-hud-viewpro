#include "gimbal.h"

extern "C" {
#include "ViewLink.h"
}

#include <cstring>
#include <mutex>
#include <atomic>

#include <QDebug>

namespace gimbal {

// ============================================================================
// Глобальные callback'и и данные
// ============================================================================
static TelemetryCallback g_telemetryCallback;
static ConnectionCallback g_connectionCallback;
static DeviceConfigCallback g_deviceConfigCallback;
static std::mutex g_callbackMutex;

static Telemetry g_currentTelemetry;
static std::mutex g_telemetryMutex;

static DeviceConfig g_currentConfig;
static std::mutex g_configMutex;
static std::atomic<bool> g_configValid{false};

// ============================================================================
// Callback'и от SDK
// ============================================================================
static int connectionCallback(int iConnStatus, const char* szMessage, int iMsgLen, void* pUserParam) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    if (g_connectionCallback) {
        ConnectionStatus status = ConnectionStatus::Unknown;
        switch (iConnStatus) {
            case VLK_CONN_STATUS_TCP_CONNECTED:
                status = ConnectionStatus::TcpConnected;
                break;
            case VLK_CONN_STATUS_TCP_DISCONNECTED:
                status = ConnectionStatus::TcpDisconnected;
                break;
            case VLK_CONN_STATUS_SERIAL_PORT_CONNECTED:
                status = ConnectionStatus::SerialPortConnected;
                break;
            case VLK_CONN_STATUS_SERIAL_PORT_DISCONNECTED:
                status = ConnectionStatus::SerialPortDisconnected;
                break;
            default:
                // Для UDP используем статус по умолчанию
                status = ConnectionStatus::Unknown;
                break;
        }
        g_connectionCallback(status);
    }
    return 0;
}

static int deviceStatusCallback(int iType, const char* szBuffer, int iBufLen, void* pUserParam) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);

    if (iType == VLK_DEV_STATUS_TYPE_TELEMETRY && iBufLen == sizeof(VLK_DEV_TELEMETRY)) {
        auto* telemetry = reinterpret_cast<const VLK_DEV_TELEMETRY*>(szBuffer);

        Telemetry t;
        t.yaw = telemetry->dYaw;
        t.pitch = telemetry->dPitch;
        t.roll = telemetry->dRoll;
        t.sensorType = static_cast<SensorType>(telemetry->emSensorType);
        t.trackerStatus = static_cast<TrackerStatus>(telemetry->emTrackerStatus);
        t.targetLat = telemetry->dTargetLat;
        t.targetLng = telemetry->dTargetLng;
        t.targetAlt = telemetry->dTargetAlt;
        t.eoDigitalZoom = telemetry->iEODigitalZoom;
        t.irDigitalZoom = telemetry->iIRDigitalZoom;
        t.zoomMagTimes = telemetry->dZoomMagTimes;
        t.laserDistance = telemetry->sLaserDistance;
        t.irColor = static_cast<IrColor>(telemetry->emIRColor);
        t.recordMode = static_cast<RecordMode>(telemetry->emRecordMode);

        {
            std::lock_guard<std::mutex> lock2(g_telemetryMutex);
            g_currentTelemetry = t;
        }

        if (g_telemetryCallback) {
            g_telemetryCallback(t);
        }
    }
    else if (iType == VLK_DEV_STATUS_TYPE_CONFIG && iBufLen == sizeof(VLK_DEV_CONFIG)) {
        auto* config = reinterpret_cast<const VLK_DEV_CONFIG*>(szBuffer);

        DeviceConfig dc;
        dc.timeZone = config->cTimeZone;
        dc.osdCfg = config->cOSDCfg;
        dc.magneticVariation = config->cMagneticVariation;
        dc.osdInput = config->cOSDInput;
        dc.baudRate = config->cBaudRate;
        dc.eoDigitalZoom = config->cEODigitalZoom;
        dc.temperatureAlarmLine = config->sTemperatureAlarmLine;
        dc.track = config->cTrack;
        dc.laser = config->cLaser;
        dc.recordDefinition = config->cRecordDefinition;
        dc.osdGps = config->cOSDGPS;
        dc.sbusChnlMap = config->cSBUSChnlMap;
        dc.chnlsMap.yaw = config->ChnlsMap.cYW;
        dc.chnlsMap.pitch = config->ChnlsMap.cPT;
        dc.chnlsMap.mode = config->ChnlsMap.cMO;
        dc.chnlsMap.zoom = config->ChnlsMap.cZM;
        dc.chnlsMap.focus = config->ChnlsMap.cFC;
        dc.chnlsMap.record = config->ChnlsMap.cRP;
        dc.chnlsMap.track = config->ChnlsMap.cMU;
        dc.modelCode = config->cCameraType;
        dc.versionNo = std::string(config->cVersionNO, 20);
        dc.deviceId = std::string(config->cDeviceID, 10);
        dc.serialNo = std::string(config->cSerialNO, 22);

        {
            std::lock_guard<std::mutex> lock2(g_configMutex);
            g_currentConfig = dc;
            g_configValid = true;
        }

        if (g_deviceConfigCallback) {
            g_deviceConfigCallback(dc);
        }
    }
    else if (iType == VLK_DEV_STATUS_TYPE_AISTATE && iBufLen >= sizeof(int)) {
        auto* aiState = reinterpret_cast<const int*>(szBuffer);
        AiStatus status = static_cast<AiStatus>(*aiState);
        // AI статус можно обработать при необходимости
        (void)status;
    }

    return 0;
}

// ============================================================================
// Вспомогательные функции конвертации
// ============================================================================
static VLK_OSD_PARAM toVlkOSDParam(const OSDParam& param) {
    VLK_OSD_PARAM vlkParam{};
    vlkParam.cOSD = param.osd;
    vlkParam.cOSDInput = param.osdInput;
    return vlkParam;
}

static VLK_CHANNELS_MAP toVlkChannelsMap(const ChannelsMap& map) {
    VLK_CHANNELS_MAP vlkMap{};
    vlkMap.cYW = map.yaw;
    vlkMap.cPT = map.pitch;
    vlkMap.cMO = map.mode;
    vlkMap.cZM = map.zoom;
    vlkMap.cFC = map.focus;
    vlkMap.cRP = map.record;
    vlkMap.cMU = map.track;
    return vlkMap;
}

// ============================================================================
// Gimbal::Impl
// ============================================================================
struct Gimbal::Impl {
    bool initialized = false;
};

// ============================================================================
// Конструктор / Деструктор
// ============================================================================
Gimbal::Gimbal()
    : m_impl(new Impl())
{
}

Gimbal::~Gimbal() {
    disconnect();
    delete m_impl;
}

// ============================================================================
// Инициализация SDK
// ============================================================================
bool Gimbal::init() {
    int result = VLK_Init();
    return (result == VLK_ERROR_NO_ERROR);
}

void Gimbal::shutdown() {
    VLK_UnInit();
}

const char* Gimbal::getSdkVersion() {
    return GetSDKVersion();
}

// ============================================================================
// Подключение и отключение
// ============================================================================
bool Gimbal::connect(const ConnectionConfig& config) {
    VLK_CONN_PARAM connParam{};

    switch (config.type) {
        case ConnectionType::Tcp: {
            connParam.emType = VLK_CONN_TYPE_TCP;
            strncpy(connParam.ConnParam.IPAddr.szIPV4, config.ip.c_str(), 15);
            connParam.ConnParam.IPAddr.szIPV4[15] = '\0';
            connParam.ConnParam.IPAddr.iPort = config.port;
            break;
        }
        case ConnectionType::Udp: {
            connParam.emType = VLK_CONN_TYPE_UDP;
            strncpy(connParam.ConnParam.IPAddr.szIPV4, config.ip.c_str(), 15);
            connParam.ConnParam.IPAddr.szIPV4[15] = '\0';
            connParam.ConnParam.IPAddr.iPort = config.port;
            break;
        }
        case ConnectionType::SerialPort: {
            connParam.emType = VLK_CONN_TYPE_SERIAL_PORT;
            strncpy(connParam.ConnParam.SerialPort.szSerialPortName, config.serialPort.c_str(), 15);
            connParam.ConnParam.SerialPort.szSerialPortName[15] = '\0';
            connParam.ConnParam.SerialPort.iBaudRate = config.baudRate;
            break;
        }
    }

    // Регистрируем callback'и перед подключением
    VLK_RegisterDevStatusCB(deviceStatusCallback, nullptr);

    int result = VLK_Connect(&connParam, connectionCallback, nullptr);
    return (result == VLK_ERROR_NO_ERROR);
}

void Gimbal::disconnect() {
    VLK_Disconnect();
}

void Gimbal::disconnectTcp() {
    VLK_DisconnectTCP();
}

void Gimbal::disconnectSerialPort() {
    VLK_DisconnectSerialPort();
}

bool Gimbal::isConnected() const {
    return VLK_IsConnected();
}

bool Gimbal::isTCPConnected() const {
    return VLK_IsTCPConnected();
}

bool Gimbal::isSerialPortConnected() const {
    return VLK_IsSerialPortConnected();
}

bool Gimbal::isUDPConnected() const {
    // Функция отсутствует в текущей версии SDK
    return false;
}

// ============================================================================
// Callback'и
// ============================================================================
void Gimbal::setTelemetryCallback(TelemetryCallback cb) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_telemetryCallback = std::move(cb);
}

void Gimbal::setConnectionCallback(ConnectionCallback cb) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_connectionCallback = std::move(cb);
}

void Gimbal::setDeviceConfigCallback(DeviceConfigCallback cb) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_deviceConfigCallback = std::move(cb);
}

// ============================================================================
// Настройки соединения
// ============================================================================
void Gimbal::setKeepAliveInterval(int ms) {
    VLK_SetKeepAliveInterval(ms);
}

// ============================================================================
// Управление движением (Pan/Tilt)
// ============================================================================
void Gimbal::move(int16_t yawSpeed, int16_t pitchSpeed) {
    static int moveCount = 0;
    static int lastLoggedMove = 0;
    moveCount++;
    
    // Логируем каждый 50-й вызов move или при изменении направления
    if (moveCount - lastLoggedMove >= 50) {
        qDebug().nospace() << "[GIMBAL] -> VLK_Move(" << yawSpeed << ", " << pitchSpeed << ") call #" << moveCount;
        lastLoggedMove = moveCount;
    }
    
    VLK_Move(yawSpeed, pitchSpeed);
}

void Gimbal::stop() {
    static int stopCount = 0;
    static int lastLoggedStop = 0;
    stopCount++;
    
    // Логируем каждый 10-й вызов stop для отладки
    if (stopCount - lastLoggedStop >= 10) {
        qDebug().nospace() << "[GIMBAL] -> VLK_Stop() call #" << stopCount;
        lastLoggedStop = stopCount;
    }
    
    VLK_Stop();
}

void Gimbal::turnTo(double yaw, double pitch) {
    VLK_TurnTo(yaw, pitch);
}

void Gimbal::home() {
    VLK_Home();
}

// ============================================================================
// Зум и фокус
// ============================================================================
void Gimbal::zoomIn(uint8_t speed) {
    VLK_ZoomIn(static_cast<short>(speed));
}

void Gimbal::zoomOut(uint8_t speed) {
    VLK_ZoomOut(static_cast<short>(speed));
}

void Gimbal::stopZoom() {
    VLK_StopZoom();
}

void Gimbal::zoomTo(float magnification) {
    VLK_ZoomTo(magnification);
}

void Gimbal::focusIn(uint8_t speed) {
    VLK_FocusIn(static_cast<short>(speed));
}

void Gimbal::focusOut(uint8_t speed) {
    VLK_FocusOut(static_cast<short>(speed));
}

void Gimbal::stopFocus() {
    VLK_StopFocus();
}

FocusMode Gimbal::getFocusMode() const {
    return static_cast<FocusMode>(VLK_GetFocusMode());
}

void Gimbal::setFocusMode(FocusMode mode) {
    VLK_SetFocusMode(static_cast<VLK_FOCUS_MODE>(mode));
}

void Gimbal::switchEODigitalZoom(bool on) {
    VLK_SwitchEODigitalZoom(on ? 1 : 0);
}

void Gimbal::irDigitalZoomIn() {
    VLK_IRDigitalZoomIn(0);
}

void Gimbal::irDigitalZoomOut() {
    VLK_IRDigitalZoomOut(0);
}

// ============================================================================
// Трекинг целей
// ============================================================================
void Gimbal::startTrack() {
    VLK_StartTrack();
}

void Gimbal::stopTrack() {
    VLK_StopTrack();
}

bool Gimbal::isTracking() const {
    return VLK_IsTracking();
}

void Gimbal::trackTarget(int x, int y, int videoWidth, int videoHeight) {
    VLK_TRACK_MODE_PARAM param{};
    param.emTrackTempSize = VLK_TRACK_TEMPLATE_SIZE_AUTO;
    param.emTrackSensor = VLK_SENSOR_VISIBLE1;
    VLK_TrackTargetPositionEx(&param, x, y, videoWidth, videoHeight);
}

void Gimbal::setTrackTemplateSize(TrackTemplateSize size) {
    VLK_SetTrackTemplateSize(static_cast<VLK_TRACK_TEMPLATE_SIZE>(size));
}

void Gimbal::enableTrackMode(const TrackModeParam& param) {
    VLK_TRACK_MODE_PARAM vlkParam{};
    vlkParam.emTrackTempSize = static_cast<VLK_TRACK_TEMPLATE_SIZE>(param.templateSize);
    vlkParam.emTrackSensor = static_cast<VLK_SENSOR>(param.sensor);
    VLK_EnableTrackMode(&vlkParam);
}

void Gimbal::disableTrackMode() {
    VLK_DisableTrackMode();
}

void Gimbal::trackTargetPosition(int x, int y, int videoWidth, int videoHeight) {
    VLK_TrackTargetPosition(x, y, videoWidth, videoHeight);
}

// ============================================================================
// Запись и фото
// ============================================================================
void Gimbal::photograph() {
    VLK_Photograph();
}

void Gimbal::startRecord() {
    VLK_SwitchRecord(1);
}

void Gimbal::stopRecord() {
    VLK_SwitchRecord(0);
}

bool Gimbal::isRecording() const {
    return VLK_IsRecording();
}

RecordMode Gimbal::getRecordMode() const {
    return static_cast<RecordMode>(VLK_GetRecordMode());
}

void Gimbal::setRecordMode(RecordMode mode) {
    VLK_SetRecordMode(static_cast<VLK_RECORD_MODE>(mode));
}

// ============================================================================
// Моторы
// ============================================================================
void Gimbal::motorOn(bool on) {
    VLK_SwitchMotor(on ? 1 : 0);
}

bool Gimbal::isMotorOn() const {
    return VLK_IsMotorOn();
}

// ============================================================================
// Режим следования за дроном
// ============================================================================
void Gimbal::enableFollowMode(bool enable) {
    VLK_EnableFollowMode(enable ? 1 : 0);
}

bool Gimbal::isFollowMode() const {
    return VLK_IsFollowMode();
}

// ============================================================================
// Изображение и цвета
// ============================================================================
void Gimbal::setImageColor(ImageType type, bool enablePip, IrColor irColor) {
    VLK_SetImageColor(
        static_cast<VLK_IMAGE_TYPE>(type),
        enablePip ? 1 : 0,
        static_cast<VLK_IR_COLOR>(irColor)
    );
}

// ============================================================================
// Дефог (антизапотевание)
// ============================================================================
void Gimbal::switchDefog(bool on) {
    VLK_SwitchDefog(on ? 1 : 0);
}

bool Gimbal::isDefogOn() const {
    return VLK_IsDefogOn();
}

// ============================================================================
// Лазерный дальномер
// ============================================================================
void Gimbal::switchLaser(bool on) {
    VLK_SwitchLaser(on ? 1 : 0);
}

void Gimbal::laserSingle() {
    VLK_LaserSingle();
}

void Gimbal::laserZoomIn(uint8_t speed) {
    VLK_LaserZoomIn(static_cast<short>(speed));
}

void Gimbal::laserZoomOut(uint8_t speed) {
    VLK_LaserZoomOut(static_cast<short>(speed));
}

void Gimbal::laserStopZoom() {
    VLK_LaserStopZoom();
}

void Gimbal::setLaserZoomMode(LaserZoomMode mode) {
    VLK_SetLaserZoomMode(static_cast<VLK_LASER_ZOOM_MODE>(mode));
}

// ============================================================================
// AI функции
// ============================================================================
void Gimbal::startAi() {
    VLK_StartAI();
}

void Gimbal::stopAi() {
    VLK_StopAI();
}

void Gimbal::enableAiState(bool enable) {
    if (enable) {
        VLK_OpenAiState();
    } else {
        VLK_CloseAiState();
    }
}

// ============================================================================
// GPS наведение
// ============================================================================
void Gimbal::pointCameraHere(double lat, double lng, double alt) {
    VLK_PointCameraHere(lat, lng, alt);
}

// ============================================================================
// Калибровка
// ============================================================================
void Gimbal::gyroCal() {
    VLK_GyroCal();
}

// ============================================================================
// OSD (On-Screen Display)
// ============================================================================
void Gimbal::setOSD(const OSDParam& param) {
    VLK_OSD_PARAM vlkParam = toVlkOSDParam(param);
    VLK_SetOSD(&vlkParam);
}

void Gimbal::setOSDLeft(const OSDParam& param) {
    VLK_OSD_PARAM vlkParam = toVlkOSDParam(param);
    VLK_SetOSDLeft(&vlkParam);
}

void Gimbal::setOSDRight(const OSDParam& param) {
    VLK_OSD_PARAM vlkParam = toVlkOSDParam(param);
    VLK_SetOSDRight(&vlkParam);
}

// ============================================================================
// Устройство и конфигурация
// ============================================================================
void Gimbal::queryDevConfiguration() {
    VLK_QueryDevConfiguration();
}

std::optional<DeviceInfo> Gimbal::getDeviceInfo() const {
    std::lock_guard<std::mutex> lock(g_configMutex);
    if (g_configValid) {
        DeviceInfo info;
        info.modelCode = g_currentConfig.modelCode;
        info.versionNo = g_currentConfig.versionNo;
        info.deviceId = g_currentConfig.deviceId;
        info.serialNo = g_currentConfig.serialNo;
        return info;
    }
    return std::nullopt;
}

// ============================================================================
// Беспроводное управление
// ============================================================================
void Gimbal::setWirelessCtrlChnlMap(const ChannelsMap& map) {
    VLK_CHANNELS_MAP vlkMap = toVlkChannelsMap(map);
    VLK_SetWirelessCtrlChnlMap(&vlkMap);
}

// ============================================================================
// S-BUS режим
// ============================================================================
void Gimbal::enterSBUSMode() {
    VLK_EnterSBUSMode();
}

void Gimbal::exitSBUSMode() {
    VLK_ExitSBUSMode();
}

// ============================================================================
// Расширенные команды
// ============================================================================
void Gimbal::sendExtentCmd(const char* cmd, int len) {
    VLK_SendExtentCmd(cmd, len);
}

// ============================================================================
// Получение телеметрии
// ============================================================================
Telemetry Gimbal::getTelemetry() const {
    std::lock_guard<std::mutex> lock(g_telemetryMutex);
    return g_currentTelemetry;
}

} // namespace gimbal
