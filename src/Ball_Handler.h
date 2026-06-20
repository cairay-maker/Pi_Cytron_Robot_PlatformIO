#ifndef BALL_HANDLER_H
#define BALL_HANDLER_H

#include <Arduino.h>
#include <Servo.h>
#include "Data_Structures.h"

const int PIN_GRIP_ARM  = 6;  // GP6: Ball Handler Lift Arm; lower value lifts up, higher value puts it down
const int PIN_GRIP_CLAW = 7;  // GP7: Ball Handler Claw Jaws: lower value closes, higher value open

struct GripperJoint {
    uint8_t pin;       // Dedicated tracking pin
    int minAngle;
    int maxAngle;
    int defaultAngle;
    int currentAngle;
    Servo instance;
};

class Ball_Handler {
public:
    Ball_Handler();
    void begin();
    void processRescueTask(SubStatus sub);
    void handleDebugSerial(String joint, String value);

    void setGripperArm(int angle);
    void setGripperClaw(int angle);

    // --- Action Placeholders for Autonomous Routine ---
    void lowerArm();
    void liftArm();
    void openClaw();
    void closeClaw();

    void moveJointSafely(GripperJoint& joint, int targetAngle);

private:
    GripperJoint _gripArm;
    GripperJoint _gripClaw;
};

#endif