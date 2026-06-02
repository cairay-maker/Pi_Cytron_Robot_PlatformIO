#ifndef BALL_HANDLER_H
#define BALL_HANDLER_H

#include <Arduino.h>
#include "Data_Structures.h"

class Ball_Handler {
public:
    Ball_Handler();
    void begin();
    void processRescueTask(SubStatus sub);
};

#endif