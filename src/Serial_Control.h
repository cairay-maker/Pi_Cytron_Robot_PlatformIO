#ifndef SERIAL_CONTROL_H
#define SERIAL_CONTROL_H

#include <Arduino.h>
#include "Camera_Mount.h"
#include "Ball_Handler.h"
#include "Motor_Control.h"

class Serial_Control {
public:
    Serial_Control(Camera_Mount& camMount, Ball_Handler& ballHandler, Motor_Control& motors, bool& simMode, bool& requireStartButton, bool& enablePrint, bool& motorEnable);
    void begin();
    void update();
    void processCommand(String debugCmd);
    
    bool isVirtualStartPressed();
    bool isVirtualStopPressed();

private:
    Camera_Mount& _camMount;
    Ball_Handler& _ballHandler;
    Motor_Control& _motors;

    bool& _simMode;
    bool& _requireStartButton;
    bool& _enablePrint;
    bool& _motorEnable;
    
    bool _virtualStartTrigger = false;
    bool _virtualStopTrigger  = false;
};

#endif