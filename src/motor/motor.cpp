#include <Arduino.h>
#include "motor.h"
#include "app_types.h"

void motorTask(void *pvParameters)
{
    AppContext *app_ctx = static_cast<AppContext *>(pvParameters);

    pinMode(PIN_STBY, OUTPUT);
    digitalWrite(PIN_STBY, LOW); // ASAP OFF!!!

    // pwm setup
    ledcSetup(PWM_CHANNEL_1, PWM_FREQ, PWM_RES);
    ledcSetup(PWM_CHANNEL_2, PWM_FREQ, PWM_RES);

    ledcAttachPin(PIN_AIN1, PWM_CHANNEL_1);
    ledcAttachPin(PIN_AIN2, PWM_CHANNEL_2);

    // unfortunately these are stuck on either high or low
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);

    // deactivate all
    digitalWrite(PIN_BIN1, HIGH); // only way to deactive these is to put into active braking mode
    digitalWrite(PIN_BIN2, HIGH);

    ledcWrite(PWM_CHANNEL_1, 0);
    ledcWrite(PWM_CHANNEL_2, 0);

    digitalWrite(PIN_STBY, HIGH); // safe to activate motor driver now - both motors turned off, cruising mode is *never* used

    // Ok, safe to proceed
    MainEvent mainEvt = MotorsInitialised;
    xQueueSend(app_ctx->mainQueue, &mainEvt, portMAX_DELAY);

    MotorEvent evt;

    for (;;)
    {
        // should implement some sort of fail safe, but trust the brain to NOT hang
        if (!xQueueReceive(app_ctx->motorQueue, &evt, portMAX_DELAY))
            continue;

        // ---- Turning
        if (evt == TurnACW || evt == TurnCW)
        {
            // right motor broken so pivot about that - put into active braking mode
            digitalWrite(PIN_BIN1, HIGH);
            digitalWrite(PIN_BIN2, HIGH);

            // turn direction controlled by spin direction of left wheel
            if (evt == TurnACW)
            {
                ledcWrite(PWM_CHANNEL_1, MOTOR_TURN_SPEED);
                ledcWrite(PWM_CHANNEL_2, 0);
            }
            else if (evt == TurnCW)
            {

                ledcWrite(PWM_CHANNEL_1, 0);
                ledcWrite(PWM_CHANNEL_2, MOTOR_TURN_SPEED);
            }
        }
        // ----- Active braking
        else if (evt == ActiveBrake)
        {
            digitalWrite(PIN_BIN1, HIGH);
            digitalWrite(PIN_BIN2, HIGH);
            ledcWrite(PWM_CHANNEL_1, MOTOR_PWM_HIGH);
            ledcWrite(PWM_CHANNEL_2, MOTOR_PWM_HIGH);
        }
        // ----- Driving straight
        else if (evt == DriveStraight)
        {
            // set both to low, technically BIN1 is HIGH already but the glitch handles that for us!
            digitalWrite(PIN_BIN1, LOW);
            digitalWrite(PIN_BIN2, LOW);

            ledcWrite(PWM_CHANNEL_1, 0);
            ledcWrite(PWM_CHANNEL_2, MOTOR_STRAIGHT_SPEED);
        }
    }

    vTaskDelete(NULL);
}