#include "Line_Processor.h"
#include "Config.h"

Line_Processor::Line_Processor(RPi_Interface& rpi) : _rpi(rpi) {
    _kp = DEFAULT_LINE_KP;
    _kd = DEFAULT_LINE_KD;
    _baseSpeed = BASE_TRACKING_SPEED;
}

void Line_Processor::setPID(float kp, float kd, int baseSpeed) {
    _kp = kp;
    _kd = kd;
    _baseSpeed = baseSpeed;
    Serial.print(">>> Line Processor Updated -> Kp: "); Serial.print(_kp);
    Serial.print(" | Kd: "); Serial.print(_kd);
    Serial.print(" | Base: "); Serial.println(_baseSpeed);
}

MotorOutput Line_Processor::compute(SystemState currentState, SubStatus currentSubState) {
    MotorOutput output = {0, 0};

    // Only compute PD if we are actually tracing a line
    if (currentState != SystemState::LINE_TRACING) {
        return output;
    }

    int rawError = _rpi.getPixelError();
    int greenCode = _rpi.getGreenCode();

    // 1. Process Green Square Logic (Debouncing)
    if (greenCode == 1) { // Left
        _consecutiveGreenLeftFrames++;
        _consecutiveGreenRightFrames = 0;
    } else if (greenCode == 2) { // Right
        _consecutiveGreenRightFrames++;
        _consecutiveGreenLeftFrames = 0;
    } else { // None or U-Turn
        _consecutiveGreenLeftFrames = 0;
        _consecutiveGreenRightFrames = 0;
        _activeTurnBias = 0; // Clear the bias
    }

    // 2. Apply Turning Bias if verified
    if (_consecutiveGreenLeftFrames >= GREEN_DEBOUNCE_FRAMES) {
        _activeTurnBias = -GREEN_TURN_BIAS; // Force a strong negative (left) error
    } else if (_consecutiveGreenRightFrames >= GREEN_DEBOUNCE_FRAMES) {
        _activeTurnBias = GREEN_TURN_BIAS;  // Force a strong positive (right) error
    }

    // 3. Combine Vision Error with Intentional Turn Bias
    int effectiveError = rawError + _activeTurnBias;

    // Constrain error to reasonable limits so the robot doesn't overreact wildly
    effectiveError = constrain(effectiveError, -LINE_MAX_PIXEL_OFFSET, LINE_MAX_PIXEL_OFFSET);

    // 4. PD Calculation
    float derivative = effectiveError - _lastError;
    float correction = (_kp * effectiveError) + (_kd * derivative);

    // Update history
    _lastError = effectiveError;

    // 5. Calculate Final Speeds
    // If Error is Positive (Line is to the Right): Correction is Positive.
    // Right Motor needs to slow down, Left Motor needs to speed up.
    output.leftSpeed = _baseSpeed + (int)correction;
    output.rightSpeed = _baseSpeed - (int)correction;

    // Ensure we don't send illegal PWM values (0-255 is normal, we allow mild reverse for tight tracing)
    output.leftSpeed = constrain(output.leftSpeed, -50, 255);
    output.rightSpeed = constrain(output.rightSpeed, -50, 255);

    return output;
}