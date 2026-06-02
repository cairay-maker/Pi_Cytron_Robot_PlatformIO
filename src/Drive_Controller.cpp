#include "Drive_Controller.h"

Drive_Controller::Drive_Controller(Motor_Control& motorRef) : _motors(motorRef) {}

void Drive_Controller::executeTrajectory(SystemState mainState, SubStatus subState) {
    if (mainState == SystemState::IDLE_STANDBY || mainState == SystemState::CRITICAL_FAULT) {
        _motors.stop();
        return;
    }

    if (mainState == SystemState::LINE_TRACING) {
        switch (subState) {
            case SubStatus::TRACE_NORMAL:
                // Base balanced tracking speed
                _motors.setSpeed(60, 60, 60, 60);
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