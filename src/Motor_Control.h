#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
#include <Arduino.h>

struct MotorPins {
    int pos;  // PWM forward pin
    int neg;  // PWM backward pin
    int dir;  // 1 = normal, -1 = reversed (physical orientation)
};

class Motor_Control {
public:
    void begin();
    void setSpeed(int fl, int fr, int rl, int rr);
    void stop();

private:
    //                pos  neg  dir
    MotorPins FL = {  14,  15,   1 };  // Front Left
    MotorPins FR = {  13,  12,  -1 };  // Front Right
    MotorPins RL = {  10,  11,   1 };  // Rear Left
    MotorPins RR = {   9,   8,  -1 };  // Rear Right
    MotorPins* motors[4] = {&FL, &FR, &RL, &RR};

    void writeMotor(MotorPins& motor, int speed);
};

#endif