#ifndef ROBOT_STATUS_H
#define ROBOT_STATUS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Tier 1: Primary high-level operational modes (LED 1)
enum class SystemState {
    BOOT_DIAGNOSTIC,
    IDLE,
    RUNNING,
    CRITICAL_FAULT
};

// Tier 2: Modular sub-statuses or hardware errors (LED 2)
enum class SubStatus {
    NONE_OK,            // Everything normal
    ERROR_IMU,          // IMU initialization failed
    ERROR_TOF,          // Time of Flight sensor down
    ERROR_BOTH_SENSORS, // Both critical sensors down
    RUN_TURNING_LEFT,   // Robot actively executing a left sweep
    RUN_TURNING_RIGHT,  // Robot actively executing a right sweep
    RUN_LINE_LOST       // Main line dropped out of camera frame
};

class RobotStatus {
public:
    RobotStatus(uint8_t pin = 23, uint8_t numLeds = 2);
    
    void begin();
    void setSystemState(SystemState state);
    void setSubStatus(SubStatus sub);
    void update(); // Handles non-blocking blink rhythms

private:
    Adafruit_NeoPixel _pixels;
    SystemState _mainState;
    SubStatus _subStatus;
    
    unsigned long _lastToggleTime;
    bool _flashToggleState;
    
    void setPixelColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
};

#endif