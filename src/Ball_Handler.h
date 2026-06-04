#ifndef BALL_HANDLER_H
#define BALL_HANDLER_H

#include <Arduino.h>
#include <Servo.h>
#include "Data_Structures.h"

const int PIN_GRIP_ARM  = 4;  // GP4: Ball Handler Lift Arm
const int PIN_GRIP_CLAW = 5;  // GP5: Ball Handler Claw Jaws

struct GripperJoint {
    uint8_t pin;
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

    void setGripperArm(String position);  // "ARM_UP", "ARM_DOWN", "ARM_STOWED"
    void setGripperClaw(String action);   // "CLAW_OPEN", "CLAW_CLOSE", "CLAW_LOOSE"

    void moveJointSafely(GripperJoint& joint, int targetAngle);

private:
    GripperJoint _gripArm;
    GripperJoint _gripClaw;
};

#endif