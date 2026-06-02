#ifndef DRIVE_CONTROLLER_H
#define DRIVE_CONTROLLER_H

#include <Arduino.h>
#include "Motor_Control.h"
#include "Data_Structures.h"

class Drive_Controller {
public:
    Drive_Controller(Motor_Control& motorRef);
    void executeTrajectory(SystemState mainState, SubStatus subState);

private:
    Motor_Control& _motors;
};

#endif