#ifndef ROBOT_STATUS_H
#define ROBOT_STATUS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Data_Structures.h"

class Robot_Status {
public:
    // Configured for the Cytron Motion 2350 Pro: NeoPixel data pin is GP23 driving 2 LEDs
    Robot_Status(uint8_t pin = 23, uint8_t numLeds = 2);
    
    void begin();
    void update(); // Must be called continuously inside loop() to keep animations non-blocking
    
    // Core State Command Configuration Transaction Method
    void changeState(SystemState mainState, SubStatus sub = SubStatus::NONE_OK);
    
    // Read-Only State Query Getters for Muscles / Controllers
    SystemState getState() const { return _mainState; }
    SubStatus getSubStatus() const { return _subStatus; }

private:
    Adafruit_NeoPixel _pixels;
    SystemState _mainState;
    SubStatus _subStatus;
    
    unsigned long _lastToggleTime;
    bool _flashToggleState;
    
    // Internal hardware paint translation wrapper method
    void setPixelColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
};

#endif