#include "Ball_Handler.h"

Ball_Handler::Ball_Handler() {
    _gripArm.pin = PIN_GRIP_ARM;
    _gripArm.minAngle = 20;
    _gripArm.maxAngle = 160;
    _gripArm.defaultAngle = 90; // Adjust these defaults once you find your ideal resting angles
    _gripArm.currentAngle = 90;

    _gripClaw.pin = PIN_GRIP_CLAW;
    _gripClaw.minAngle = 10;
    _gripClaw.maxAngle = 170;
    _gripClaw.defaultAngle = 90; 
    _gripClaw.currentAngle = 90;
}

void Ball_Handler::begin() {
    // Force clean orientation sweep to default locations right out of the gate, then sleep
    moveJointSafely(_gripArm, _gripArm.defaultAngle);
    moveJointSafely(_gripClaw, _gripClaw.defaultAngle);
    
    Serial.println("Ball_Handler: Gripper servos initialized and sleeping quietly.");
}

void Ball_Handler::processRescueTask(SubStatus sub) {
    switch (sub) {
        case SubStatus::RESCUE_COLLECTING_BALL:
            // Example workflow for Daniel & Lexi when they automate this:
            // setGripperClaw("CLAW_OPEN");
            // setGripperArm("ARM_DOWN");
            // setGripperClaw("CLAW_CLOSE");
            // setGripperArm("ARM_UP");
            break;
            
        case SubStatus::RESCUE_DUMPING_BALL:
            // setGripperArm("ARM_DOWN");
            // setGripperClaw("CLAW_OPEN");
            // setGripperArm("ARM_STOWED");
            break;
            
        default:
            break;
    }
}

void Ball_Handler::handleDebugSerial(String joint, String value) {
    // Both strings are converted to uppercase and trimmed inside main.cpp
    
    Serial.print("Ball Handler Intercept -> Joint: "); Serial.print(joint);
    Serial.print(" | Value: "); Serial.println(value);

    if (joint == "ARM") {
        if (value == "ARM_UP" || value == "ARM_DOWN" || value == "ARM_STOWED") {
            setGripperArm(value);
        } else {
            // If it's not a text preset, convert the string to a raw number and move!
            int customAngle = value.toInt();
            moveJointSafely(_gripArm, customAngle);
        }
    } 
    else if (joint == "CLAW") {
        if (value == "CLAW_OPEN" || value == "CLAW_CLOSE" || value == "CLAW_LOOSE") {
            setGripperClaw(value);
        } else {
            // Convert the string to a raw number and move!
            int customAngle = value.toInt();
            moveJointSafely(_gripClaw, customAngle);
        }
    }
}

void Ball_Handler::setGripperArm(String position) {
    if (position == "ARM_UP") {
        moveJointSafely(_gripArm, 140);
    } else if (position == "ARM_DOWN") {
        moveJointSafely(_gripArm, 60);
    } else if (position == "ARM_STOWED") {
        moveJointSafely(_gripArm, _gripArm.defaultAngle);
    } else {
        moveJointSafely(_gripArm, position.toInt());
    }
}

void Ball_Handler::setGripperClaw(String action) {
    if (action == "CLAW_OPEN") {
        moveJointSafely(_gripClaw, 160);
    } else if (action == "CLAW_CLOSE") {
        moveJointSafely(_gripClaw, 30);
    } else if (action == "CLAW_LOOSE") {
        moveJointSafely(_gripClaw, 90);
    } else {
        moveJointSafely(_gripClaw, action.toInt());
    }
}

void Ball_Handler::moveJointSafely(GripperJoint& joint, int targetAngle) {
    // 1. Establish structural boundaries guardrail
    int safeAngle = constrain(targetAngle, joint.minAngle, joint.maxAngle);
    joint.currentAngle = safeAngle;
    
    // 2. Power up the signal bus
    joint.instance.attach(joint.pin);
    
    // 3. Write target command path
    joint.instance.write(safeAngle);
    
    // 4. Block long enough for physical components to reach target orientation
    delay(250); 
    
    // 5. Instantly float/relax the motor to stop electrical hums and vibrations
    joint.instance.detach();
}