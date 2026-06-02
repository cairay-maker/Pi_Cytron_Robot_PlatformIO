#ifndef SIMULATION_Map_H
#define SIMULATION_Map_H

#include <Arduino.h>
#include "Data_Structures.h"
#include "Robot_Status.h"

// Easily add new test cases here
enum class TestScenario {
    FULL_COMPETITION_RUN,
    RAMP_CHALLENGE,
    RESCUE_ZONE_BALL_SORT
};

class Simulation_Map {
public:
    Simulation_Map(Robot_Status& statusRef);
    
    // Choose which track layout scenario you want to run
    void initScenario(TestScenario scenario, unsigned long startTime);
    
    // Feeds virtual telemetry into the robot status manager based on the clock
    void update(unsigned long currentTime);

private:
    Robot_Status& _status;
    TestScenario _activeScenario;
    unsigned long _simStartTime;

    // Separate cleaner methods for individual test matrix paths
    void runFullCompetitionTimeline(unsigned long progress);
    void runRampChallengeTimeline(unsigned long progress);
    void runRescueZoneBallTimeline(unsigned long progress);
};

#endif