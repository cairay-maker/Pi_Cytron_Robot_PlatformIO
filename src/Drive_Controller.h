#ifndef DRIVE_CONTROLLER_H
#define DRIVE_CONTROLLER_H

#include <Arduino.h>
#include "Motor_Control.h"
#include "IMU_Control.h"
#include "Data_Structures.h"

class Drive_Controller {
public:
    Drive_Controller(Motor_Control& motorRef, IMU_Control& imuRef);
    
    // Add an overload to allow Line_Processor to feed direct motor speeds
    void executeTrajectory(SystemState mainState, SubStatus subState, int overrideSpeedL = 0, int overrideSpeedR = 0);
    
    bool executeAvoidanceManeuver(bool avoidRight); // Changed to return a success/abort flag

private:
    Motor_Control& _motors;
    IMU_Control& _imu;
    bool turnByIMU(float targetAngleDelta);
    bool safeDelay(unsigned long ms);
};

#endif