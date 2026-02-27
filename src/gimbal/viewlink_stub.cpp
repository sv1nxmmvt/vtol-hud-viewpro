// Stub implementations for missing CSerialPortConnection methods
// These are required by libViewLink.so but not implemented in the binary

extern "C" {
    // Заглушки для недостающих методов CSerialPortConnection
    // Символы с name mangling для g++
    
    void _ZN21CSerialPortConnection10SetOSDLeftEhh(unsigned char, unsigned char) {}
    void _ZN21CSerialPortConnection11SetOSDRightEhh(unsigned char, unsigned char) {}
    void _ZN21CSerialPortConnection13StartCarTrackEv() {}
    void _ZN21CSerialPortConnection17SetGimbalAimPointEddd(double, double, double) {}
    void _ZN21CSerialPortConnection6StopAIEv() {}
    void _ZN21CSerialPortConnection7StartAIEv() {}
    void _ZN21CSerialPortConnection7GyroCalEv() {}
}
