#pragma once
#include <Arduino.h>

// Structural System States (LED 1)
enum class SystemState {
    BOOT_DIAGNOSTIC,
    IDLE_STANDBY,
    LINE_TRACING,
    RESCUE_ZONE,
    CRITICAL_FAULT
};

// Specialized Behavioral Sub-Actions (LED 2)
enum class SubStatus {
    NONE_OK,
    ERROR_IMU,
    ERROR_TOF,
    
    // Line Tracing Contexts
    TRACE_NORMAL,
    TRACE_SHARP_TURN_BRAKE,  // The champion-tier backup/brake maneuver
    TRACE_GREEN_LEFT,
    TRACE_GREEN_RIGHT,
    TRACE_GREEN_UTURN,
    TRACE_GAP,
    TRACE_RAMP_UP,
    TRACE_RAMP_DOWN,
    TRACE_OBSTACLE_AVOIDANCE,
    
    // Rescue Zone Contexts
    RESCUE_ENTER_ZONE,
    RESCUE_SCANNING_BALLS,
    RESCUE_COLLECTING_BALL,
    RESCUE_DUMPING_BALL,
    RESCUE_EXITING
};

// Existing Sensor Data Packages
struct struct_tof_target {
    uint16_t distance;
    uint8_t  status;
    float    signalRate;
    float    ambientRate;
};

struct struct_tof_data {
    uint8_t           count;
    struct_tof_target targets[4];
    unsigned long     timestamp;
};

struct struct_imu_data {
    float yaw; float pitch; float roll;
    float accX; float accY; float accZ;
    float gyroX; float gyroY; float gyroZ;
    float accuracy;
};