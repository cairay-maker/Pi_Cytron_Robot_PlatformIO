#include "Simulation_Map.h"

Simulation_Map::Simulation_Map(Robot_Status& statusRef) 
    : _status(statusRef), _activeScenario(TestScenario::FULL_COMPETITION_RUN), _simStartTime(0) {}

void Simulation_Map::initScenario(TestScenario scenario, unsigned long startTime) {
    _activeScenario = scenario;
    _simStartTime = startTime;
}

void Simulation_Map::update(unsigned long currentTime) {
    unsigned long progress = currentTime - _simStartTime;

    switch (_activeScenario) {
        case TestScenario::FULL_COMPETITION_RUN:
            runFullCompetitionTimeline(progress);
            break;
        case TestScenario::RAMP_CHALLENGE:
            runRampChallengeTimeline(progress);
            break;
        case TestScenario::RESCUE_ZONE_BALL_SORT:
            runRescueZoneBallTimeline(progress);
            break;
    }
}

// === SCENARIO 1: The standard diverse track run ===
void Simulation_Map::runFullCompetitionTimeline(unsigned long progress) {
    if (progress < 3000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
    } 
    else if (progress >= 3000 && progress < 5000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_GREEN_RIGHT);
    } 
    else if (progress >= 5000 && progress < 7000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_SHARP_TURN_BRAKE); // Champion reverse
    } 
    else if (progress >= 7000 && progress < 10000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_OBSTACLE_AVOIDANCE);
    } 
    else if (progress >= 10000) {
        _status.changeState(SystemState::RESCUE_ZONE, SubStatus::RESCUE_SCANNING_BALLS);
    }
}

// === SCENARIO 2: Pure torque-calibration ramp stress test ===
void Simulation_Map::runRampChallengeTimeline(unsigned long progress) {
    if (progress < 4000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
    } 
    else if (progress >= 4000 && progress < 8000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_RAMP_UP); // Test high torque speeds
    } 
    else if (progress >= 8000 && progress < 12000) {
        _status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_RAMP_DOWN); // Test crawl braking
    } 
    else {
        _status.changeState(SystemState::IDLE_STANDBY, SubStatus::NONE_OK);
    }
}

// === SCENARIO 3: Direct-to-rescue payload test ===
void Simulation_Map::runRescueZoneBallTimeline(unsigned long progress) {
    if (progress < 3000) {
        _status.changeState(SystemState::RESCUE_ZONE, SubStatus::RESCUE_ENTER_ZONE);
    } 
    else if (progress >= 3000 && progress < 6000) {
        _status.changeState(SystemState::RESCUE_ZONE, SubStatus::RESCUE_COLLECTING_BALL); // Arm drop phase
    } 
    else if (progress >= 6000 && progress < 9000) {
        _status.changeState(SystemState::RESCUE_ZONE, SubStatus::RESCUE_DUMPING_BALL); // Sorting gate phase
    } 
    else {
        _status.changeState(SystemState::RESCUE_ZONE, SubStatus::RESCUE_EXITING);
    }
}