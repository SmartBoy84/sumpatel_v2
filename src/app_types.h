#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_MCP23008.h>

#define SECOND_CORE 0

#define mainQueue_n 1 // adjust if need more - right now, only motor communicates
#define displayQueue_n 1
#define brainQueue_n 1 // sensor overwrites old one
#define motorQueue_n 1

enum MainEvent
{
    MotorsInitialised
};

enum DisplayEvent
{
    SensorsInitialising,
    CountDown1,
    CountDown2,
    Start,

    // brain states
    DisplayTurning, // blue
    DisplayStraight, // green
    DisplayStopped, // red
    DisplayReadjusting // purple
    
};

typedef struct
{
    uint32_t front_centre_dist;
    uint32_t front_left_dist;
    uint32_t front_right_dist;
} UltrasonicDistanceFrame;

typedef struct
{
    bool front_centre;
    bool front_left;
    bool front_right;
    bool rear_right;
    bool rear_left;
} IrDetectionFrame;

typedef struct
{
    UltrasonicDistanceFrame us;
    IrDetectionFrame ir;

} BrainFrame;

// no speed control either unfortunately...
enum MotorEvent
{
    DriveStraight,
    ActiveBrake,
    TurnCW,
    TurnACW,
    // Reverse // sad...
};

typedef struct
{
    TFT_eSPI *tft;
    Adafruit_MCP23008 *mcp;

    // task queues
    QueueHandle_t mainQueue;    // for tasks to communicate status to main (setup() or loop())
    QueueHandle_t displayQueue; // brain -> display
    QueueHandle_t brainQueue;   // sensor -> brain
    QueueHandle_t motorQueue;   // brain -> motor
} AppContext;

#endif