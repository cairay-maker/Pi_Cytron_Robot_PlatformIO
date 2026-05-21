#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include <Arduino.h>

class Button_Control {
public:
    Button_Control(int startPin, int stopPin);
    void begin();
    bool isStartPressed();  // rising edge only
    bool isStopPressed();   // rising edge only
private:
    int _startPin;
    int _stopPin;
    int _prevStart = HIGH;
    int _prevStop  = HIGH;
};

#endif