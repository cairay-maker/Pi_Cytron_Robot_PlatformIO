#ifndef LINE_PROCESSOR_H
#define LINE_PROCESSOR_H

#include <Arduino.h>
#include "RPi_Interface.h"
#include "Data_Structures.h"

struct MotorOutput {
    int leftSpeed;
    int rightSpeed;
};

class Line_Processor {
public:
    Line_Processor(RPi_Interface& rpi);
    
    // Updates the tuning values (can be called from Serial_Control)
    void setPID(float kp, float kd, int baseSpeed);
    
    // Calculates the required motor speeds based on RPi Vision data
    MotorOutput compute(SystemState currentState, SubStatus currentSubState);

private:
    RPi_Interface& _rpi;
    
    float _kp;
    float _kd;
    int   _baseSpeed;
    
    int _lastError = 0;
    
    // Green Square Anti-Confusion Variables
    int _consecutiveGreenLeftFrames = 0;
    int _consecutiveGreenRightFrames = 0;
    int _activeTurnBias = 0;
};

#endif