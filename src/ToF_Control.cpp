#include "ToF_Control.h"
#include "Config.h"

// Constructed globally before setup() — critical for the VL53L4CX underlying library
// Binds to Wire (GP16/17) using default shutdown pin parameters (-1)
VL53L4CX _tof_sensor_instance(&Wire, -1);

ToF_Control::ToF_Control() {}

bool ToF_Control::begin(TwoWire& wire) {
    // Note: wire.begin() is already executed once in main.cpp setup()
    // Running an active I2C address scan before InitSensor can corrupt the VL53L4CX internal boot sequence.
    
    Serial.println("ToF: Initializing sensor layout via address 0x29...");
    if (_tof_sensor_instance.InitSensor(0x29) != 0) {
        Serial.println("ToF: InitSensor hardware register handshake failed!");
        return false;
    }
    
    // Fire up continuous autonomous ranging cycles
    _tof_sensor_instance.VL53L4CX_StartMeasurement();
    Serial.println("ToF: Ranging measurement engine active.");
    return true;
}

bool ToF_Control::read(struct_tof_data& data) {
    uint8_t isReady = 0;
    
    // Non-blocking register polling to check if the sensor has finished an active data harvest
    _tof_sensor_instance.VL53L4CX_GetMeasurementDataReady(&isReady);

    if (isReady) {
        VL53L4CX_MultiRangingData_t results;
        
        // Pull raw ranging matrices directly from the device registers
        if (_tof_sensor_instance.VL53L4CX_GetMultiRangingData(&results) == 0) {
            // Constrain object count to prevent array out-of-bounds errors (Max 4 tracking profiles)
            data.count = results.NumberOfObjectsFound;
            if (data.count > 4) data.count = 4;
            
            for (uint8_t i = 0; i < data.count; i++) {
                data.targets[i].distance    = results.RangeData[i].RangeMilliMeter;
                data.targets[i].status      = results.RangeData[i].RangeStatus; // 0 = Clean valid read
                data.targets[i].signalRate  = results.RangeData[i].SignalRateRtnMegaCps;
                data.targets[i].ambientRate = results.RangeData[i].AmbientRateRtnMegaCps;
            }
            data.timestamp = millis();
        }
        
        // Clear the device interrupt register flag to cleanly trigger the next laser cycle path emission
        _tof_sensor_instance.VL53L4CX_ClearInterruptAndStartMeasurement();
        return true;
    }
    
    return false; // Return false smoothly if data isn't ready yet, preventing loop lag
}

bool ToF_Control::evaluateObstacle(const struct_tof_data& data, unsigned long currentTime, unsigned long lastAvoidanceEndTime) {
    // If we are in the 2.5s blind cooldown period, ignore targets and reset confidence
    if (currentTime - lastAvoidanceEndTime <= 2500) {
        _confidenceCount = 0;
        return false;
    }

    if (data.count > 0) {
        int16_t currentDistance = data.targets[0].distance;
        
        // Filter out hardware errors and out-of-range anomalies (e.g., 0 or 65535)
        if (currentDistance >= TOF_MIN_VALID_DIST_MM && currentDistance <= TOF_MAX_VALID_DIST_MM) {
            if (currentDistance <= OBSTACLE_DISTANCE_THRESHOLD_MM) {
                _confidenceCount++;
            } else {
                _confidenceCount = 0; // Clear if path opens up
            }
        } else {
            // Ghost reading or error state: reset confidence counter
            _confidenceCount = 0; 
        }
        
        if (_confidenceCount >= OBSTACLE_DEBOUNCE_COUNT) {
            _confidenceCount = 0; // Auto-reset counter for the next obstacle
            return true;
        }
    }
    return false;
}

void ToF_Control::resetConfidence() {
    _confidenceCount = 0;
}