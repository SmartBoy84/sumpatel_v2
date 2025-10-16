#ifndef MOTORS_H
#define MOTORS_H

// pwm settings
#define PWM_RES 8       // 8 bit is enough
#define PWM_CHANNEL_1 1 // IN1
#define PWM_CHANNEL_2 2 // IN2
#define PWM_FREQ 20000

// settings
#define MOTOR_PWM_HIGH 255

#define MOTOR_TURN_SPEED 170                // scanning -> very slow - note; below 150 is stopped
#define MOTOR_STRAIGHT_SPEED MOTOR_PWM_HIGH // max - HAS to be because left is broken (always high)

// pins
#define PIN_STBY 13 // IMPORTANT, set STBY to LOW ASAP

#define PIN_AIN1 1 // PWM channel CH_A_IN1 (forward PWM)
#define PIN_AIN2 2 // PWM channel CH_A_IN2 (reverse PWM)

#define PIN_BIN1 10 // PWM channel CH_B_IN1 (forward PWM)
#define PIN_BIN2 3  // PWM channel CH_B_IN2 (reverse PWM)

void motorTask(void *pvParameters);

#endif