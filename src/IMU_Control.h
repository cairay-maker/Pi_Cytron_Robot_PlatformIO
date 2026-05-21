#ifndef IMU_CONTROL_H
#define IMU_CONTROL_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO08x.h>
#include "Data_Structures.h"

class IMU_Control {
public:
    IMU_Control();
    bool begin(TwoWire& wire);  // pass Wire or Wire1
    bool read(struct_imu_data& data);
private:
    Adafruit_BNO08x _bno;
};

#endif