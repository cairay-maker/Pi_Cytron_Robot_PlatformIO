#include "Ball_Handler.h"

Ball_Handler::Ball_Handler() {
    // Define: Pin, Min Limit, Max Limit, Default Baseline Position
    _gripArm = { PIN_GRIP_ARM, 20, 160, 45, -1 }; // -1 ensures it sweeps to default on boot
    _gripClaw = { PIN_GRIP_CLAW, 10, 170, 90, -1 };
}

void Ball_Handler::begin() {
    moveJointSafely(_gripArm, _gripArm.defaultAngle);
    moveJointSafely(_gripClaw, _gripClaw.defaultAngle);
    
    Serial.println("Ball_Handler: Gripper servos initialized and sleeping quietly.");
}

void Ball_Handler::processRescueTask(SubStatus sub) {
    switch (sub) {
        case SubStatus::RESCUE_COLLECTING_BALL:
            // Example workflow for Daniel & Lexi when they automate this:
            // openClaw();
            // lowerArm();
            // closeClaw();
            // liftArm();
            break;
            
        case SubStatus::RESCUE_DUMPING_BALL:
            // lowerArm();
            // openClaw(); // Release the ball
            // liftArm();  // Stow it back safely
            break;
            
        default:
            break;
    }
}

void Ball_Handler::handleDebugSerial(String joint, String value) {
    Serial.print("Ball Handler Intercept -> Joint: "); Serial.print(joint);
    Serial.print(" | Value: "); Serial.println(value);

    int targetAngle = value.toInt();
    Serial.print("   -> Converted to Integer: "); Serial.println(targetAngle);

    if (joint == "ARM") {
        setGripperArm(targetAngle);
    } 
    else if (joint == "CLAW") {
        setGripperClaw(targetAngle);
    }
}

void Ball_Handler::setGripperArm(int angle) {
    moveJointSafely(_gripArm, angle);
}

void Ball_Handler::setGripperClaw(int angle) {
    moveJointSafely(_gripClaw, angle);
}

// --- Descriptive Action Placeholders ---
void Ball_Handler::lowerArm()  { setGripperArm(60); }
void Ball_Handler::liftArm()   { setGripperArm(140); }
void Ball_Handler::openClaw()  { setGripperClaw(160); }
void Ball_Handler::closeClaw() { setGripperClaw(30); }

void Ball_Handler::moveJointSafely(GripperJoint& joint, int targetAngle) {
    int safeAngle = constrain(targetAngle, joint.minAngle, joint.maxAngle);
    if (safeAngle == joint.currentAngle) return; // Skip redundant moves to prevent loop delays

    Serial.print(">>> Executing Move: Servo Pin "); Serial.print(joint.pin);
    Serial.print(" -> Target Angle: "); Serial.println(safeAngle);

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