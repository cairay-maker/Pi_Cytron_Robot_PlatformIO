#include "Camera_Mount.h"

Camera_Mount::Camera_Mount() {
    // Define: Pin, Min Limit, Max Limit, Default Baseline Position
    _neck = { PIN_CAM1_NECK,  30, 150,  90,  90 }; 
    _base = { PIN_CAM1_BASE,  10, 170,  90,  90 }; 
}

void Camera_Mount::begin() {
    // We do NOT permanently attach here anymore.
    // Instead, we let moveJointSafely handle it on demand.
    moveJointSafely(_neck, _neck.defaultAngle);
    moveJointSafely(_base, _base.defaultAngle);
    
    Serial.println("Camera_Mount: Pan/Tilt Servos Initialized (Sleeping).");
}

void Camera_Mount::moveJointSafely(MountJoint& joint, int targetAngle) {
    int safeAngle = constrain(targetAngle, joint.minAngle, joint.maxAngle);
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

void Camera_Mount::setNeck(String position) {
    if (position == "LOOK_DOWN")  moveJointSafely(_neck, 45);  // Close look for tight curves
    if (position == "LOOK_UP")    moveJointSafely(_neck, 120); // Look ahead for ramps
    if (position == "LOOK_LEVEL") moveJointSafely(_neck, 90);  // Normal tracing
}

void Camera_Mount::setBase(String position) {
    if (position == "PAN_LEFT")   moveJointSafely(_base, 45);
    if (position == "PAN_RIGHT")  moveJointSafely(_base, 135);
    if (position == "PAN_CENTER") moveJointSafely(_base, 90);
}

// === Automatic Gaze Shifting ===
void Camera_Mount::adjustGazeForState(SystemState mainState, SubStatus sub) {
    if (mainState == SystemState::LINE_TRACING) {
        switch (sub) {
            case SubStatus::TRACE_RAMP_UP:
                setNeck("LOOK_UP"); // Look uphill so it doesn't get blinded by the incline
                break;
            case SubStatus::TRACE_SHARP_TURN_BRAKE:
                setNeck("LOOK_DOWN"); // Look tight at the wheels to find a lost line
                break;
            default:
                setNeck("LOOK_LEVEL");
                setBase("PAN_CENTER");
                break;
        }
    }
}

void Camera_Mount::handleDebugSerial(String joint, String value) {
    if (joint == "NECK") {
        if (value == "LOOK_DOWN" || value == "LOOK_UP" || value == "LOOK_LEVEL") setNeck(value);
        else moveJointSafely(_neck, value.toInt());
    } 
    else if (joint == "BASE") {
        if (value == "PAN_LEFT" || value == "PAN_RIGHT" || value == "PAN_CENTER") setBase(value);
        else moveJointSafely(_base, value.toInt());
    }
}