#include "Camera_Mount.h"

Camera_Mount::Camera_Mount() {
    // Define: Pin, Min Limit, Max Limit, Default Baseline Position
    _pan = { PIN_CAM1_PAN,  10, 170,  90,  -1 }; // -1 ensures it sweeps to default on boot; lower number pans right, higher number pans right
    _tilt = { PIN_CAM1_TILT,  10, 150,  10,  -1 }; // -1 ensures it sweeps to default on boot; lower number lift up, higher number look down
}

void Camera_Mount::begin() {
    // We do NOT permanently attach here anymore.
    // Instead, we let moveJointSafely handle it on demand.
    moveJointSafely(_pan, _pan.defaultAngle);
    moveJointSafely(_tilt, _tilt.defaultAngle);
    
    Serial.println("Camera_Mount: Pan/Tilt Servos Initialized (Sleeping).");
}

void Camera_Mount::moveJointSafely(MountJoint& joint, int targetAngle) {
    int safeAngle = constrain(targetAngle, joint.minAngle, joint.maxAngle);
    if (safeAngle == joint.currentAngle) return; // Skip if already there to prevent loop delays

    Serial.print(">>> Executing Move: Servo Pin "); Serial.print(joint.pin);
    Serial.print(" -> Target Angle: "); Serial.println(safeAngle);

    joint.currentAngle = safeAngle;
    
    // 1. Wake up the muscle channel
    joint.instance.attach(joint.pin); 
    
    // 2. Command the movement
    joint.instance.write(safeAngle);   
    
    // 3. Give the physical gears time to complete the sweep
    // (150ms-250ms is the sweet spot for a standard 90-degree MG90S travel)
    delay(200); 
    
    // 4. Cut the power pulse to make it dead silent!
    joint.instance.detach();           
}

void Camera_Mount::setPan(int angle) {
    moveJointSafely(_pan, angle);
}

void Camera_Mount::setTilt(int angle) {
    moveJointSafely(_tilt, angle);
}

void Camera_Mount::handleDebugSerial(String joint, String value) {
    Serial.print("Camera Mount Intercept -> Joint: "); Serial.print(joint);
    Serial.print(" | Value: "); Serial.println(value);

    int targetAngle = value.toInt();
    Serial.print("   -> Converted to Integer: "); Serial.println(targetAngle);

    if (joint == "TILT") {
        setTilt(targetAngle);
    } 
    else if (joint == "PAN") {
        setPan(targetAngle);
    }
}