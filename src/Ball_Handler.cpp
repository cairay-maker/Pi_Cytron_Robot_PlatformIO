#include "Ball_Handler.h"

Ball_Handler::Ball_Handler() {}

void Ball_Handler::begin() {
    // Initialize sorting servos and payload sensor ports down the road
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