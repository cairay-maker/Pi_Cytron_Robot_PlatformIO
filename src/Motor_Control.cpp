#include "Motor_Control.h"

void Motor_Control::begin() {
    for (int i = 0; i < 4; i++) {
        pinMode(motors[i]->pos, OUTPUT);
        pinMode(motors[i]->neg, OUTPUT);
    }
    stop();
    Serial.println("Motor_Control: initialized and stopped");
}

void Motor_Control::writeMotor(MotorPins& motor, int speed) {
    // Apply direction correction for physically reversed motors
    int corrected = constrain(speed * motor.dir, -255, 255);
    if (corrected > 0) {
        analogWrite(motor.pos, corrected);
        analogWrite(motor.neg, 0);
    } else if (corrected < 0) {
        analogWrite(motor.pos, 0);
        analogWrite(motor.neg, -corrected);
    } else {
        analogWrite(motor.pos, 0);
        analogWrite(motor.neg, 0);
    }
}

void Motor_Control::setSpeed(int fl, int fr, int rl, int rr) {
    int speeds[4] = {fl, fr, rl, rr};
    for (int i = 0; i < 4; i++) writeMotor(*motors[i], speeds[i]);
}

void Motor_Control::stop() {
    setSpeed(0, 0, 0, 0);
}