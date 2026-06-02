#include "Robot_Status.h"

RobotStatus::RobotStatus(uint8_t pin, uint8_t numLeds) 
    : _pixels(numLeds, pin, NEO_GRB + NEO_KHZ800), 
      _mainState(SystemState::BOOT_DIAGNOSTIC),
      _subStatus(SubStatus::NONE_OK),
      _lastToggleTime(0),
      _flashToggleState(false) {}

void RobotStatus::begin() {
    _pixels.begin();
    _pixels.setBrightness(40);
    _pixels.show();
}

void RobotStatus::setSystemState(SystemState state) {
    if (_mainState != state) {
        _mainState = state;
    }
}

void RobotStatus::setSubStatus(SubStatus sub) {
    if (_subStatus != sub) {
        _subStatus = sub;
    }
}

void RobotStatus::setPixelColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(index, _pixels.Color(r, g, b));
}

void RobotStatus::update() {
    unsigned long currentTime = millis();
    // 200ms interval for fast diagnostic / error blinking
    if (currentTime - _lastToggleTime >= 200) {
        _flashToggleState = !_flashToggleState;
        _lastToggleTime = currentTime;
    }

    // ==========================================
    // LAYER 1: PRIMARY STATUS (LED 1 - Index 0)
    // ==========================================
    switch (_mainState) {
        case SystemState::BOOT_DIAGNOSTIC:
            // Soft white/yellow while performing diagnostic routines
            setPixelColorRGB(0, 100, 80, 0); 
            break;
            
        case SystemState::IDLE:
            setPixelColorRGB(0, 0, 0, 255); // Solid Blue = Thinking / Standby
            break;
            
        case SystemState::RUNNING:
            setPixelColorRGB(0, 0, 255, 0); // Solid Green = Executing code cleanly
            break;
            
        case SystemState::CRITICAL_FAULT:
            if (_flashToggleState) {
                setPixelColorRGB(0, 255, 0, 0); // Flashing Red = Total Stop Required
            } else {
                setPixelColorRGB(0, 0, 0, 0);
            }
            break;
    }

    // ==========================================
    // LAYER 2: SUB-STATUS / DIAGNOSTICS (LED 2 - Index 1)
    // ==========================================
    switch (_subStatus) {
        case SubStatus::NONE_OK:
            // In boot mode, solid green means "Diagnostics Clear"
            if (_mainState == SystemState::BOOT_DIAGNOSTIC) {
                setPixelColorRGB(1, 0, 255, 0); 
            } else {
                setPixelColorRGB(1, 0, 0, 0); // Off during normal execution loops
            }
            break;

        case SubStatus::ERROR_IMU:
            // Solid Magenta = IMU broken
            setPixelColorRGB(1, 255, 0, 255); 
            break;

        case SubStatus::ERROR_TOF:
            // Solid Orange/Amber = ToF broken
            setPixelColorRGB(1, 255, 100, 0); 
            break;

        case SubStatus::ERROR_BOTH_SENSORS:
            if (_flashToggleState) {
                setPixelColorRGB(1, 255, 0, 0); // Flashing Red on LED 2 = Multiple device failure
            } else {
                setPixelColorRGB(1, 0, 0, 0);
            }
            break;

        case SubStatus::RUN_TURNING_LEFT:
            // Cyan pulse/blink on the left side to show active trajectory logic
            if (_flashToggleState) setPixelColorRGB(1, 0, 255, 255);
            else setPixelColorRGB(1, 0, 0, 0);
            break;

        case SubStatus::RUN_TURNING_RIGHT:
            // Purple pulse/blink on the right side
            if (_flashToggleState) setPixelColorRGB(1, 128, 0, 128);
            else setPixelColorRGB(1, 0, 0, 0);
            break;

        case SubStatus::RUN_LINE_LOST:
            setPixelColorRGB(1, 255, 255, 0); // Solid Yellow = Caution, searching for track
            break;
    }

    _pixels.show();
}