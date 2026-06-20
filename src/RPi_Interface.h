#ifndef RPI_INTERFACE_H
#define RPI_INTERFACE_H

#include <Arduino.h>

class RPi_Interface {
public:
    RPi_Interface();
    void begin();
    void update();

    // The core Vision Data outputs
    int getPixelError();
    int getGreenCode();
    bool isDataFresh();

    // Manual Overrides
    void setMockVision(int error, int greenCode);

private:
    // Initial boot state values. 
    // Setting these to 0 assumes the robot starts perfectly centered on a straight line.
    int _currentError = 0;
    int _currentGreenCode = 0;
    bool _isFresh = false;
    
    // Non-blocking serial parsing buffer
    static const int MAX_PACKET_LEN = 32;
    char _buffer[MAX_PACKET_LEN];
    int _bufIndex = 0;

    void parsePacket();
};

#endif