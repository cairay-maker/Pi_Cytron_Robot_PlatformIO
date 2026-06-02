#include "Robot_Status.h"

// Scope constructor updated to match your exact class naming convention
Robot_Status::Robot_Status(uint8_t pin, uint8_t numLeds) 
    : _pixels(numLeds, pin, NEO_GRB + NEO_KHZ800), 
      _mainState(SystemState::BOOT_DIAGNOSTIC), 
      _subStatus(SubStatus::NONE_OK),
      _lastToggleTime(0), 
      _flashToggleState(false) {}

void Robot_Status::begin() {
    _pixels.begin();
    _pixels.setBrightness(40); // Optimized brightness to save battery bus current draw
    _pixels.show();
}

// Global Single Source of Truth transaction window to change states together cleanly
void Robot_Status::changeState(SystemState mainState, SubStatus sub) {
    _mainState = mainState;
    _subStatus = sub;
}

void Robot_Status::setPixelColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(index, _pixels.Color(r, g, b));
}

void Robot_Status::update() {
    unsigned long currentTime = millis();
    // 200ms non-blocking blink interval for active alerts/turns
    if (currentTime - _lastToggleTime >= 200) {
        _flashToggleState = !_flashToggleState;
        _lastToggleTime = currentTime;
    }

    // ==========================================
    // LAYER 1: MASTER MODE TRACKING (LED 1 - Index 0)
    // ==========================================
    switch (_mainState) {
        case SystemState::BOOT_DIAGNOSTIC:   
            setPixelColorRGB(0, 120, 80, 0); // Solid Yellow = Hardware Audit
            break;
            
        case SystemState::IDLE_STANDBY:      
            setPixelColorRGB(0, 0, 0, 255);  // Solid Blue = Standby / Ready
            break;
            
        case SystemState::LINE_TRACING:      
            setPixelColorRGB(0, 0, 255, 0);  // Solid Green = Normal Automation Trace
            break;
            
        case SystemState::RESCUE_ZONE:       
            setPixelColorRGB(0, 255, 0, 255); // Solid Magenta = Victim Retrieval Active
            break;
            
        case SystemState::CRITICAL_FAULT:
            if (_flashToggleState) {
                setPixelColorRGB(0, 255, 0, 0); // Flashing Red = Emergency Hardware Stop
            } else {
                setPixelColorRGB(0, 0, 0, 0);
            }
            break;
    }

    // ==========================================
    // LAYER 2: FINE BEHAVIOR DETAILS (LED 2 - Index 1)
    // ==========================================
    switch (_subStatus) {
        case SubStatus::NONE_OK:
            if (_mainState == SystemState::BOOT_DIAGNOSTIC) {
                setPixelColorRGB(1, 0, 255, 0); // Clear Boot Confirmation Green
            } else {
                setPixelColorRGB(1, 0, 0, 0);   // Dark during normal balanced routing
            }
            break;

        case SubStatus::ERROR_IMU:            
            setPixelColorRGB(1, 255, 0, 0);     // Solid Red Overlay = Missing I2C IMU
            break;

        case SubStatus::ERROR_TOF:            
            setPixelColorRGB(1, 255, 128, 0);   // Solid Orange Overlay = Missing I2C ToF
            break;
            
        case SubStatus::TRACE_NORMAL:
            setPixelColorRGB(1, 0, 200, 100);   // Solid Teal = Normal trace execution
            break;
            
        case SubStatus::TRACE_SHARP_TURN_BRAKE: 
            setPixelColorRGB(1, 255, 0, 100);   // Pink/Magenta = Running Champion Braking/Backup Sequence
            break;
            
        case SubStatus::TRACE_GREEN_LEFT:     
            setPixelColorRGB(1, 0, 255, 255);   // Solid Cyan = Approaching Left Turn Indicator
            break;
            
        case SubStatus::TRACE_GREEN_RIGHT:    
            setPixelColorRGB(1, 128, 0, 128);   // Solid Purple = Approaching Right Turn Indicator
            break;
            
        case SubStatus::TRACE_GREEN_UTURN:
            setPixelColorRGB(1, 0, 128, 255);   // Solid Light Blue = U-Turn execution
            break;
            
        case SubStatus::TRACE_GAP:
            setPixelColorRGB(1, 200, 200, 0);   // Solid Olive = Gap/ditch crossing
            break;
            
        case SubStatus::TRACE_RAMP_UP:
            setPixelColorRGB(1, 255, 165, 0);   // Solid Orange = Ramping upward
            break;
            
        case SubStatus::TRACE_RAMP_DOWN:
            setPixelColorRGB(1, 255, 69, 0);    // Solid Orange-Red = Ramping downward
            break;
            
        case SubStatus::TRACE_OBSTACLE_AVOIDANCE: 
            setPixelColorRGB(1, 255, 100, 0);   // Amber = Routing Around Obstacle Core Cylinder
            break;
            
        case SubStatus::RESCUE_ENTER_ZONE:
            setPixelColorRGB(1, 200, 0, 200);   // Purple = Entering rescue zone
            break;
            
        case SubStatus::RESCUE_SCANNING_BALLS:
            setPixelColorRGB(1, 100, 255, 0);   // Lime Green = Scanning for balls
            break;
            
        case SubStatus::RESCUE_COLLECTING_BALL: 
            if (_flashToggleState) {
                setPixelColorRGB(1, 255, 255, 255); // Flashing White = Sorting Claw/Gimbal deployment active
            } else {
                setPixelColorRGB(1, 0, 0, 0);
            }
            break;
            
        case SubStatus::RESCUE_DUMPING_BALL:
            setPixelColorRGB(1, 255, 0, 255);   // Magenta = Dumping ball
            break;
            
        case SubStatus::RESCUE_EXITING:
            setPixelColorRGB(1, 255, 165, 100); // Coral = Exiting rescue zone
            break;

        default:                              
            setPixelColorRGB(1, 0, 0, 0); 
            break;
    }

    _pixels.show(); // Flush changes down the serial hardware data pin (GP23)
}