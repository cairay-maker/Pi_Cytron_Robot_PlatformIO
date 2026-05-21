#include "ToF_Control.h"

// Constructed globally before setup() — critical for VL53L4CX library
// Currently on Wire (GP16/17). Change to &Wire1 if swapping ports.
VL53L4CX _tof_sensor_instance(&Wire, -1);

ToF_Control::ToF_Control() {}

bool ToF_Control::begin(TwoWire& wire) {
    // Wire already initialized in main.cpp — do NOT call wire.begin() here
    // Do NOT run I2C scan before InitSensor — corrupts VL53L4CX boot state

    Serial.println("ToF: InitSensor(0x29)...");
    if (_tof_sensor_instance.InitSensor(0x29) != 0) {
        Serial.println("ToF: InitSensor failed!");
        return false;
    }
    _tof_sensor_instance.VL53L4CX_StartMeasurement();
    Serial.println("ToF: Ready");
    return true;
}

bool ToF_Control::read(struct_tof_data& data) {
    uint8_t isReady = 0;
    _tof_sensor_instance.VL53L4CX_GetMeasurementDataReady(&isReady);

    if (isReady) {
        VL53L4CX_MultiRangingData_t results;
        if (_tof_sensor_instance.VL53L4CX_GetMultiRangingData(&results) == 0) {
            data.count = results.NumberOfObjectsFound;
            for (uint8_t i = 0; i < data.count && i < 4; i++) {
                data.targets[i].distance    = results.RangeData[i].RangeMilliMeter;
                data.targets[i].status      = results.RangeData[i].RangeStatus;
                data.targets[i].signalRate  = results.RangeData[i].SignalRateRtnMegaCps;
                data.targets[i].ambientRate = results.RangeData[i].AmbientRateRtnMegaCps;
            }
            data.timestamp = millis();
        }
        _tof_sensor_instance.VL53L4CX_ClearInterruptAndStartMeasurement();
        return true;
    }
    return false;
}