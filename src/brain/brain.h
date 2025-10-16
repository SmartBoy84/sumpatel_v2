#ifndef BRAIN_H
#define BRAIN_H

#include <Arduino.h>
#include "app_types.h"

#define COUNTDOWN_BUTTON BUTTON_1

#define ACTIVE_BRAKE_MS 1000          // ms
#define TARGET_DISTANCE_THRESHOLD 60 // cm
#define DEVIATION_THRESHOLD 20       // front/right deviate by 20 cm, so turn and recalibrate
#define MAX_READJUSTMENT_TIME_MS 10000

// gotta keep track between updates
enum BotState
{
    Stopped,
    LineTurning,
    ScanTurning,
    Straight
};

void brainTask(void *pvParameters);

#endif