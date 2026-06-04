#include "Ball_Handler.h"

Ball_Handler::Ball_Handler() {
    _gripArm.pin = PIN_GRIP_ARM;
    _gripArm.minAngle = 20;
    _gripArm.maxAngle = 160;
    _gripArm.defaultAngle = 90;
    _gripArm.currentAngle = 90;

    _gripClaw.pin = PIN_GRIP_CLAW;
    _gripClaw.minAngle = 10;
    _gripClaw.maxAngle = 170;
    _gripClaw.defaultAngle = 90;
    _gripClaw.currentAngle = 90;
}

void Ball_Handler::begin() {
    _gripArm.instance.attach(_gripArm.pin);
    _gripClaw.instance.attach(_gripClaw.pin);
    moveJointSafely(_gripArm, _gripArm.defaultAngle);
    moveJointSafely(_gripClaw, _gripClaw.defaultAngle);
    Serial.println("Ball_Handler: gripper servos ready.");
}

void Ball_Handler::processRescueTask(SubStatus sub) {
    // Placeholders for your future mechanical arm deployment routines
    switch (sub) {
        case SubStatus::RESCUE_COLLECTING_BALL:
            // deployClawLower();
            break;
        case SubStatus::RESCUE_DUMPING_BALL:
            // openSortingGate();
            break;
        default:
            break;
    }
}

void Ball_Handler::handleDebugSerial(String joint, String value) {
    if (joint == "ARM") {
        setGripperArm(value);
    } else if (joint == "CLAW") {
        setGripperClaw(value);
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
    int safeAngle = constrain(targetAngle, joint.minAngle, joint.maxAngle);
    joint.currentAngle = safeAngle;
    joint.instance.write(safeAngle);
}
