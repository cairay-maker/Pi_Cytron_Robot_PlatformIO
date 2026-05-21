#pragma once
#include <Arduino.h>

// --- ToF Target ---
struct struct_tof_target {
    uint16_t distance;      // mm
    uint8_t  status;        // 0 = valid
    float    signalRate;    // Mcps
    float    ambientRate;   // Mcps
};

// --- ToF Data ---
struct struct_tof_data {
    uint8_t           count;        // number of objects found (max 4)
    struct_tof_target targets[4];   // up to 4 objects
    unsigned long     timestamp;    // millis() of last valid read
};

// --- IMU Data ---
struct struct_imu_data {
    float yaw;        // rotation vector real component
    float pitch;      // rotation vector i
    float roll;       // rotation vector j
    float accX;       // m/s²
    float accY;
    float accZ;
    float gyroX;      // rad/s
    float gyroY;
    float gyroZ;
    float accuracy;   // rotation vector accuracy
};