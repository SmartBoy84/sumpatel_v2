#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

#define ROUND_ROBIN_MS 50 // wait 50ms before testing other ultrasonic
#define SOUND_SPEED 0.034
#define NO_US_DIST 0 // if 0, assume broken

// ultrasonic pins
#define TRIG_F 16
#define ECHO_F 17

#define TRIG_FL 43
#define ECHO_FL 18

#define TRIG_FR 44
#define ECHO_FR 21

// IR sensors
#define IR_FL_MCP 1
#define IR_RL_MCP 3
#define IR_RR_MCP 4
#define IR_FR_MCP 2
#define IR_FC_MCP 0

// Expander settings
#define WireClk 100000

#define I2C_SDA 11
#define I2C_SCL 12
#define MCP_ADDR 0x20

void sensorsTask(void *pvParameters);
void ultrasonicTask(void *pvParameters);

#endif