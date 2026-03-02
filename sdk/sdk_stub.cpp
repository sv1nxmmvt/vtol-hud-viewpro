/**
 * Заглушка для отсутствующих функций в libViewLink.so
 * Эти функции должны быть предоставлены SDK, но отсутствуют в текущей версии
 */

#include <cstdint>

// Класс-заглушка для CSerialPortConnection с правильным name mangling
class CSerialPortConnection {
public:
    static void SetOSDLeft(unsigned char, unsigned char);
    static void SetOSDRight(unsigned char, unsigned char);
    static void StartAI();
    static void StopAI();
    static void SetGimbalAimPoint(double, double, double);
    static void StartCarTrack();
    static void GyroCal();
};

// Реализация методов
void CSerialPortConnection::SetOSDLeft(unsigned char, unsigned char) {}
void CSerialPortConnection::SetOSDRight(unsigned char, unsigned char) {}
void CSerialPortConnection::StartAI() {}
void CSerialPortConnection::StopAI() {}
void CSerialPortConnection::SetGimbalAimPoint(double, double, double) {}
void CSerialPortConnection::StartCarTrack() {}
void CSerialPortConnection::GyroCal() {}

// Заглушки для отсутствующих функций SDK
extern "C" {
    void VLK_QueryDevConfiguration() {}
    void VLK_ZoomTo(float) {}
    int VLK_GetFocusMode() { return 0; }
    void VLK_SetFocusMode(int) {}
    void VLK_SwitchEODigitalZoom(int) {}
    void VLK_IRDigitalZoomIn(short) {}
    void VLK_IRDigitalZoomOut(short) {}
    int VLK_GetRecordMode() { return 0; }
    void VLK_SetRecordMode(int) {}
    int VLK_IsDefogOn() { return 0; }
    void VLK_SwitchDefog(int) {}
    void VLK_LaserSingle() {}
    void VLK_LaserZoomIn(short) {}
    void VLK_LaserZoomOut(short) {}
    void VLK_LaserStopZoom() {}
    void VLK_SetLaserZoomMode(int) {}
    void VLK_SetOSD(const void*) {}
    void VLK_SetOSDLeft(const void*) {}
    void VLK_SetOSDRight(const void*) {}
    void VLK_SetWirelessCtrlChnlMap(const void*) {}
    void VLK_EnterSBUSMode() {}
    void VLK_ExitSBUSMode() {}
    void VLK_SendExtentCmd(const char*, int) {}
}
