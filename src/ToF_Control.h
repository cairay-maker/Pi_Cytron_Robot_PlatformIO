#ifndef TOF_CONTROL_H
#define TOF_CONTROL_H

#include <Arduino.h>
#include <Wire.h>
#include <vl53l4cx_class.h>
#include "Data_Structures.h"

// Global instance — must be constructed before setup()
// Same pattern as working standalone code
extern VL53L4CX _tof_sensor_instance;

class ToF_Control {
public:
    ToF_Control();
    bool begin(TwoWire& wire);  // pass Wire or Wire1
    bool read(struct_tof_data& data);
    
    // Data evaluation wrapper to keep main.cpp clean
    bool evaluateObstacle(const struct_tof_data& data, unsigned long currentTime, unsigned long lastAvoidanceEndTime);
    void resetConfidence();

private:
    int _confidenceCount = 0;
};

#endif