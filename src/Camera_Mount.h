#ifndef CAMERA_MOUNT_H
#define CAMERA_MOUNT_H

#include <Arduino.h>
#include <Servo.h>
#include "Data_Structures.h"

// --- Physical Pin Assignments ---
const int PIN_CAM1_PAN = 1;  // GP1: Camera 1 Pan (Left/Right); lower number pans right, higher number pans left
const int PIN_CAM1_TILT = 2;  // GP2: Camera 1 Tilt (Up/Down); lower number lifts up, higher number looks down

struct MountJoint {
    uint8_t pin;       // Needs to be explicit
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
    
    void handleDebugSerial(String joint, String value);

    // --- Descriptive Control API Interface ---
    void setPan(int angle);
    void setTilt(int angle);
    
    void moveJointSafely(MountJoint& joint, int targetAngle);

private:
    MountJoint _pan;
    MountJoint _tilt;

};

#endif