#ifndef CAMERA_MOUNT_H
#define CAMERA_MOUNT_H

#include <Arduino.h>
#include <Servo.h>
#include "Data_Structures.h"

// --- Physical Pin Assignments ---
const int PIN_CAM1_NECK = 2;  // GP2: Camera 1 Tilt (Up/Down)
const int PIN_CAM1_BASE = 3;  // GP3: Camera 1 Pan (Left/Right)

struct MountJoint {
    uint8_t pin;
    int minAngle;
    int maxAngle;
    int defaultAngle;
    int currentAngle;
    Servo instance;
};

class Camera_Mount {
public:
    Camera_Mount();
    void begin();
    
    // Updates the camera's gaze based on what the robot is currently doing
    void adjustGazeForState(SystemState mainState, SubStatus sub);
    void handleDebugSerial(String joint, String value);

    // --- Descriptive Control API Interface ---
    void setNeck(String position);  // "LOOK_DOWN", "LOOK_UP", "LOOK_LEVEL"
    void setBase(String position);  // "PAN_LEFT", "PAN_RIGHT", "PAN_CENTER"
    
    void moveJointSafely(MountJoint& joint, int targetAngle);

private:
    MountJoint _neck;
    MountJoint _base;
};

#endif