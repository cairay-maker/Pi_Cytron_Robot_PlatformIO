#include "Drive_Controller.h"
#include "Config.h"

Drive_Controller::Drive_Controller(Motor_Control& motorRef, IMU_Control& imuRef) : _motors(motorRef), _imu(imuRef) {}

void Drive_Controller::executeTrajectory(SystemState mainState, SubStatus subState, int overrideSpeedL, int overrideSpeedR) {
    if (mainState == SystemState::IDLE_STANDBY || mainState == SystemState::CRITICAL_FAULT) {
        _motors.stop();
        return;
    }

    if (mainState == SystemState::LINE_TRACING) {
        switch (subState) {
            case SubStatus::TRACE_NORMAL:
                // Base balanced tracking speed OR override speeds from Vision Line Processor
                if (overrideSpeedL != 0 || overrideSpeedR != 0) {
                    _motors.setSpeed(overrideSpeedL, overrideSpeedR, overrideSpeedL, overrideSpeedR);
                } else {
                    _motors.setSpeed(BASE_TRACKING_SPEED, BASE_TRACKING_SPEED, BASE_TRACKING_SPEED, BASE_TRACKING_SPEED);
                }
                break;

            case SubStatus::TRACE_SHARP_TURN_BRAKE:
                // Champion Maneuver: Hard reverse to stabilize pivot tracking point
                _motors.setSpeed(-40, -40, -40, -40);
                break;

            case SubStatus::TRACE_GREEN_LEFT:
                // Pivot sharply to the left on the central axis
                _motors.setSpeed(-50, 50, -50, 50);
                break;

            case SubStatus::TRACE_GREEN_RIGHT:
                // Pivot sharply to the right on the central axis
                _motors.setSpeed(50, -50, 50, -50);
                break;

            case SubStatus::TRACE_RAMP_UP:
                // Supply higher torque current to scale the ramp incline
                _motors.setSpeed(95, 95, 95, 95);
                break;

            case SubStatus::TRACE_RAMP_DOWN:
                // Use engine braking dynamics to safely crawl down the ramp
                _motors.setSpeed(30, 30, 30, 30);
                break;

            case SubStatus::TRACE_OBSTACLE_AVOIDANCE:
                // Arc orbit path trajectory tracing profile
                _motors.setSpeed(30, 80, 30, 80);
                break;

            default:
                _motors.setSpeed(40, 40, 40, 40);
                break;
        }
    }
    
    if (mainState == SystemState::RESCUE_ZONE) {
        // Hand off control to slow, precise alignment maneuvers
        _motors.setSpeed(35, 35, 35, 35);
    }
}

bool Drive_Controller::safeDelay(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        if (digitalRead(STOP_PIN) == LOW) { // Stop button pressed
            _motors.stop();
            return false; // Signal to abort
        }
        delay(10); // Small yield to maintain responsiveness
        yield();   // CRITICAL: Keep USB alive during blocking delay
    }
    return true;
}

bool Drive_Controller::executeAvoidanceManeuver(bool avoidRight) {
    float firstTurn = avoidRight ? 90.0 : -90.0;
    float arcTurn   = avoidRight ? -180.0 : 180.0;
    float thirdTurn = avoidRight ? 90.0 : -90.0;
    
    // 1. Full Stop to kill momentum before starting the sequence
    _motors.stop();
    if (!safeDelay(500)) return false;
    
    // Step 1: The Clear Turn (90 degrees outward)
    Serial.println(">>> AVOIDANCE PHASE 1: The Clear Turn");
    if (!turnByIMU(firstTurn)) return false;
    if (!safeDelay(200)) return false; // Settle after turn
    
    // Step 2: The Differential Arc
    Serial.println(">>> AVOIDANCE PHASE 2: The Differential Arc");
    int arcSpeedL = avoidRight ? OBSTACLE_AVOID_ARC_INNER_SPEED : OBSTACLE_AVOID_ARC_OUTER_SPEED;
    int arcSpeedR = avoidRight ? OBSTACLE_AVOID_ARC_OUTER_SPEED : OBSTACLE_AVOID_ARC_INNER_SPEED;
    _motors.setSpeed(arcSpeedL, arcSpeedR, arcSpeedL, arcSpeedR);
    
    struct_imu_data imuData = {0};
    for (int i=0; i<30; i++) {
        if (digitalRead(STOP_PIN) == LOW) return false;
        _imu.read(imuData);
        delay(2);
    }
    float startYaw = imuData.yaw;
    float targetYaw = startYaw - arcTurn;
    while (targetYaw > 180.0) targetYaw -= 360.0;
    while (targetYaw < -180.0) targetYaw += 360.0;
    
    unsigned long arcStartTime = millis();
    // Increased timeout from 10s to 15s to guarantee it finishes the 180-degree sweep even on low battery
    while (millis() - arcStartTime < 15000) {
        yield(); // CRITICAL: Keep USB alive during 15-second blocking arc
        if (digitalRead(STOP_PIN) == LOW) {
            _motors.stop();
            return false;
        }
        if (_imu.read(imuData)) {
            float error = targetYaw - imuData.yaw;
            while (error > 180.0) error -= 360.0;
            while (error < -180.0) error += 360.0;
            
            // Compensate for IMU measurement delay with threshold
            if (abs(error) < OBSTACLE_AVOID_ARC_FINISH_DEG) {
                Serial.println(">>> ARC COMPLETE");
                break;
            }
        }
    }
    _motors.stop();
    if (!safeDelay(500)) return false;
    
    // Step 3: The Realignment Turn
    Serial.println(">>> AVOIDANCE PHASE 3: The Realignment Turn");
    if (!turnByIMU(thirdTurn)) return false;
    if (!safeDelay(200)) return false;

    // After realigning, resume forward line-tracing with the base tracking speed
    _motors.setSpeed(BASE_TRACKING_SPEED, BASE_TRACKING_SPEED, BASE_TRACKING_SPEED, BASE_TRACKING_SPEED);
    Serial.println(">>> AVOIDANCE COMPLETE: Resuming Line Tracking.");
    return true;
}

bool Drive_Controller::turnByIMU(float targetAngleDelta) {
    struct_imu_data imuData = {0}; // FIX: Initialize to 0 to prevent garbage memory bugs!
    
    // FIX: The IMU sends Accel, Gyro, and Rotation events sequentially. 
    // We must pump the queue enough times to guarantee we catch a Rotation event.
    for (int i=0; i<30; i++) {
        if (digitalRead(STOP_PIN) == LOW) return false;
        _imu.read(imuData);
        delay(2);
    }
    
    float startYaw = imuData.yaw;
    // FIX: Based on your physical IMU orientation, turning right (+ delta) 
    // DECREASES the yaw angle. Therefore, we subtract the delta from the current yaw.
    float targetYaw = startYaw - targetAngleDelta;
    // Normalize target to -180 / +180 boundary
    while (targetYaw > 180.0) targetYaw -= 360.0;
    while (targetYaw < -180.0) targetYaw += 360.0;
    
    Serial.print(">>> TURN INIT | Start Yaw: "); Serial.print(startYaw);
    Serial.print(" | Target Delta: "); Serial.print(targetAngleDelta);
    Serial.print(" | Target Yaw: "); Serial.println(targetYaw);

    unsigned long turnStartTime = millis();
    // Blocking turn with an 8-second timeout so a bumped wire doesn't make the robot spin forever
    while (millis() - turnStartTime < 8000) {
        yield(); // CRITICAL: Keep USB alive during 8-second blocking turn
        
        // Abort turn if STOP button is pressed
        if (digitalRead(STOP_PIN) == LOW) {
            _motors.stop();
            return false;
        }
        
        if (_imu.read(imuData)) {
            float error = targetYaw - imuData.yaw;
            while (error > 180.0) error -= 360.0;
            while (error < -180.0) error += 360.0;
            
            float absError = abs(error);
            int dirL = (error < 0) ? 1 : -1;
            int dirR = (error < 0) ? -1 : 1;
            
            int speedBrake  = OBSTACLE_AVOID_TURN_BASE_SPEED * OBSTACLE_AVOID_TURN_BRAKE_PCT;

            if (absError > OBSTACLE_AVOID_TURN_BRAKE_DEG) {
                // Approach Target
                _motors.setSpeed(OBSTACLE_AVOID_TURN_BASE_SPEED * dirL, OBSTACLE_AVOID_TURN_BASE_SPEED * dirR, OBSTACLE_AVOID_TURN_BASE_SPEED * dirL, OBSTACLE_AVOID_TURN_BASE_SPEED * dirR);
            } else {
                // Stage 3 (Active Brake Lock)
                _motors.setSpeed(speedBrake * -dirL, speedBrake * -dirR, speedBrake * -dirL, speedBrake * -dirR);
                delay(OBSTACLE_AVOID_TURN_BRAKE_MS);
                _motors.stop();
                Serial.print(">>> TURN COMPLETE | Final Yaw: "); Serial.print(imuData.yaw);
                Serial.print(" | Error: "); Serial.println(error);
                break; // Reached target within tolerance!
            }
        }
    }
    
    // Catch a timeout just in case it got stuck spinning
    if (millis() - turnStartTime >= 8000) {
        Serial.print(">>> TURN TIMEOUT (8s) | Final Yaw: "); Serial.println(imuData.yaw);
    }
    
    _motors.stop();
    return true;
}