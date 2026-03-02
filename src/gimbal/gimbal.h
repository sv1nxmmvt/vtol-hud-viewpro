#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <optional>
#include <array>

#include "gimbal_config.h"

namespace gimbal {

// ============================================================================
// Статус подключения
// ============================================================================
enum class ConnectionStatus {
    TcpConnected,
    TcpDisconnected,
    UdpConnected,
    UdpDisconnected,
    SerialPortConnected,
    SerialPortDisconnected,
    Unknown
};

// ============================================================================
// Типы сенсоров и изображения
// ============================================================================
enum class SensorType {
    Visible1 = 0,       // Видимый свет (основной)
    Ir = 1,             // ИК (основной)
    VisibleIr = 2,      // Видимый + ИК PIP
    IrVisible = 3,      // ИК + видимый PIP
    Visible2 = 4        // Видимый свет (дополнительный)
};

enum class ImageType {
    Visible1 = 0,       // Зум-объектив видимого света
    Visible2 = 1,       // Зум-объектив ИК
    Ir1 = 2,            // ИК зум-объектив
    Ir2 = 3,            // ИК доп. объектив
    Fusion = 4          // Слияние видимого + ИК
};

enum class IrColor {
    WhiteHot = 0,       // Белый горячий
    BlackHot = 1,       // Черный горячий
    PseudoHot = 2,      // Псевдогорячий
    Rusty = 3           // Ржавый
};

// ============================================================================
// Режимы работы
// ============================================================================
enum class RecordMode {
    None = 0,           // Ни фото ни запись
    Photo = 1,          // Режим фото
    Record = 2          // Режим записи
};

enum class FocusMode {
    Auto = 0,           // Автофокус
    Manu = 1            // Ручной фокус
};

enum class LaserZoomMode {
    FollowEO = 0,       // Следовать за видимым светом
    Manu = 1            // Ручное управление
};

enum class TrackTemplateSize {
    Auto = 0,           // Авто
    Size32 = 32,        // 32x32
    Size64 = 64,        // 64x64
    Size128 = 128       // 128x128
};

// ============================================================================
// Статусы трекера
// ============================================================================
enum class TrackerStatus {
    Stopped = 0,        // Остановлен
    Searching = 1,      // Поиск
    Tracking = 2,       // Слежение
    Lost = 3            // Потерян
};

// ============================================================================
// Статусы AI
// ============================================================================
enum class AiStatus {
    Doing = 0,          // AI активен
    Stop = 1            // AI остановлен
};

// ============================================================================
// Параметры режима трекинга
// ============================================================================
struct TrackModeParam {
    TrackTemplateSize templateSize = TrackTemplateSize::Auto;
    SensorType sensor = SensorType::Visible1;
};

// ============================================================================
// Телеметрия
// ============================================================================
struct Telemetry {
    double yaw = 0.0;
    double pitch = 0.0;
    double roll = 0.0;
    SensorType sensorType = SensorType::Visible1;
    TrackerStatus trackerStatus = TrackerStatus::Stopped;
    double targetLat = 0.0;
    double targetLng = 0.0;
    double targetAlt = 0.0;
    int eoDigitalZoom = 0;
    int irDigitalZoom = 0;
    double zoomMagTimes = 0.0;
    int16_t laserDistance = 0;
    IrColor irColor = IrColor::WhiteHot;
    RecordMode recordMode = RecordMode::None;
};

// ============================================================================
// Карта каналов беспроводного управления
// ============================================================================
struct ChannelsMap {
    char yaw = 0;   // Высокие 4 бита: yaw влево, низкие 4 бита: yaw вправо
    char pitch = 0; // Высокие 4 бита: pitch вверх, низкие 4 бита: pitch вниз
    char mode = 0;  // Высокие 4 бита: скорость, низкие 4 бита: центрирование
    char zoom = 0;  // Высокие 4 бита: зум уменьшение, низкие 4 бита: зум увеличение
    char focus = 0; // Высокие 4 бита: фокус наружу, низкие 4 бита: фокус внутрь
    char record = 0;// Высокие 4 бита: фото, низкие 4 бита: запись гимбала
    char track = 0; // Высокие 4 бита: старт трек, низкие 4 бита: стоп трек
};

// ============================================================================
// Конфигурация устройства
// ============================================================================
struct DeviceConfig {
    char timeZone = 0;                      // Часовой пояс
    char osdCfg = 0;                        // OSD конфигурация
    char magneticVariation = 0;             // Магнитное склонение
    char osdInput = 0;                      // OSD вход
    char baudRate = 0;                      // Baudrate гимбала (0-7)
    char eoDigitalZoom = 0;                 // Цифровой зум EO (1: вкл, 0: выкл)
    int16_t temperatureAlarmLine = 0;       // Сигнализация температуры
    char track = 0;                         // Статус трека (1: вкл, 0: выкл)
    char laser = 0;                         // Режим лазера (0-3)
    char recordDefinition = 0;              // Определение записи (1: 4K, 2: 1080P)
    char osdGps = 0;                        // OSD GPS (0: UAV, 1: цель)
    char sbusChnlMap = 0;                   // S.BUS/Mavlink каналы (1-4)
    ChannelsMap chnlsMap{};                 // Карта каналов беспроводного управления
    char modelCode = 0;                     // Код модели устройства
    std::string versionNo;                  // Версия прошивки
    std::string deviceId;                   // ID устройства
    std::string serialNo;                   // Серийный номер
};

struct DeviceInfo {
    char modelCode = 0;
    std::string modelName;
    std::string versionNo;
    std::string deviceId;
    std::string serialNo;
};

// ============================================================================
// OSD параметры
// ============================================================================
struct OSDParam {
    char osd = 0;       // Маска OSD (битовая)
    char osdInput = 0;  // Маска входа OSD (битовая)
};

// Маски OSD
enum class OsdMask : unsigned char {
    EnableOsd = 0x01,           // Включить OSD
    Cross = 0x02,               // Перекрестие в центре
    PitchYaw = 0x04,            // Показывать pitch и yaw
    XyShift = 0x08,             // Сдвиг XY
    Gps = 0x10,                 // GPS
    Time = 0x20,                // Время
    VlMag = 0x40,               // VL-MAG
    BigFont = 0x80              // Большой шрифт
};

// Маски входа OSD
enum class OsdInputMask : unsigned char {
    PermanentSave = 0x01,       // Сохранить конфигурацию
    Time = 0x02,                // Ввод времени
    Gps = 0x04,                 // GPS
    Mgrs = 0x08,                // MGRS
    PitchYaw = 0x10,            // Pitch и Yaw
    VlMag = 0x20,               // VL-MAG
    ZoomTimesOrFov = 0x40,      // Зум или FOV
    CharBlackBorder = 0x80      // Черная рамка символов
};

// ============================================================================
// Callback'и (объявления типов)
// ============================================================================
using TelemetryCallback = std::function<void(const Telemetry&)>;
using ConnectionCallback = std::function<void(ConnectionStatus status)>;
using DeviceConfigCallback = std::function<void(const DeviceConfig&)>;

// ============================================================================
// Класс Gimbal
// ============================================================================
class Gimbal {
public:
    Gimbal();
    ~Gimbal();

    // =========================================================================
    // Инициализация SDK
    // =========================================================================
    static bool init();
    static void shutdown();
    static const char* getSdkVersion();

    // =========================================================================
    // Подключение и отключение
    // =========================================================================
    bool connect(const ConnectionConfig& config);
    void disconnect();
    void disconnectTcp();
    void disconnectSerialPort();
    bool isConnected() const;
    bool isTCPConnected() const;
    bool isSerialPortConnected() const;
    bool isUDPConnected() const;

    // =========================================================================
    // Callback'и
    // =========================================================================
    void setTelemetryCallback(TelemetryCallback cb);
    void setConnectionCallback(ConnectionCallback cb);
    void setDeviceConfigCallback(DeviceConfigCallback cb);

    // =========================================================================
    // Настройки соединения
    // =========================================================================
    void setKeepAliveInterval(int ms);

    // =========================================================================
    // Управление движением (Pan/Tilt)
    // =========================================================================
    void move(int16_t yawSpeed, int16_t pitchSpeed);
    void stop();
    void turnTo(double yaw, double pitch);
    void home();

    // =========================================================================
    // Зум и фокус
    // =========================================================================
    void zoomIn(uint8_t speed);
    void zoomOut(uint8_t speed);
    void stopZoom();
    void zoomTo(float magnification);

    void focusIn(uint8_t speed);
    void focusOut(uint8_t speed);
    void stopFocus();

    FocusMode getFocusMode() const;
    void setFocusMode(FocusMode mode);

    // Цифровой зум
    void switchEODigitalZoom(bool on);
    void irDigitalZoomIn();
    void irDigitalZoomOut();

    // =========================================================================
    // Трекинг целей
    // =========================================================================
    void startTrack();
    void stopTrack();
    bool isTracking() const;
    void trackTarget(int x, int y, int videoWidth, int videoHeight);
    void setTrackTemplateSize(TrackTemplateSize size);

    // Расширенный трекинг
    void enableTrackMode(const TrackModeParam& param);
    void disableTrackMode();
    void trackTargetPosition(int x, int y, int videoWidth, int videoHeight);

    // =========================================================================
    // Запись и фото
    // =========================================================================
    void photograph();
    void startRecord();
    void stopRecord();
    bool isRecording() const;

    RecordMode getRecordMode() const;
    void setRecordMode(RecordMode mode);

    // =========================================================================
    // Моторы
    // =========================================================================
    void motorOn(bool on);
    bool isMotorOn() const;

    // =========================================================================
    // Режим следования за дроном
    // =========================================================================
    void enableFollowMode(bool enable);
    bool isFollowMode() const;

    // =========================================================================
    // Изображение и цвета
    // =========================================================================
    void setImageColor(ImageType type, bool enablePip, IrColor irColor);

    // =========================================================================
    // Дефог (антизапотевание)
    // =========================================================================
    void switchDefog(bool on);
    bool isDefogOn() const;

    // =========================================================================
    // Лазерный дальномер
    // =========================================================================
    void switchLaser(bool on);
    void laserSingle();
    void laserZoomIn(uint8_t speed);
    void laserZoomOut(uint8_t speed);
    void laserStopZoom();
    void setLaserZoomMode(LaserZoomMode mode);

    // =========================================================================
    // AI функции
    // =========================================================================
    void startAi();
    void stopAi();
    void enableAiState(bool enable);

    // =========================================================================
    // GPS наведение
    // =========================================================================
    void pointCameraHere(double lat, double lng, double alt);

    // =========================================================================
    // Калибровка
    // =========================================================================
    void gyroCal();

    // =========================================================================
    // OSD (On-Screen Display)
    // =========================================================================
    void setOSD(const OSDParam& param);
    void setOSDLeft(const OSDParam& param);
    void setOSDRight(const OSDParam& param);

    // =========================================================================
    // Устройство и конфигурация
    // =========================================================================
    void queryDevConfiguration();
    std::optional<DeviceInfo> getDeviceInfo() const;

    // =========================================================================
    // Беспроводное управление
    // =========================================================================
    void setWirelessCtrlChnlMap(const ChannelsMap& map);

    // =========================================================================
    // S-BUS режим
    // =========================================================================
    void enterSBUSMode();
    void exitSBUSMode();

    // =========================================================================
    // Расширенные команды
    // =========================================================================
    void sendExtentCmd(const char* cmd, int len);

    // =========================================================================
    // Получение телеметрии
    // =========================================================================
    Telemetry getTelemetry() const;

private:
    struct Impl;
    Impl* m_impl;
};

} // namespace gimbal
