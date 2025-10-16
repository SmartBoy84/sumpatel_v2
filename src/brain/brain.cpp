#include <Arduino.h>

#include "brain.h"
#include "app_types.h"
#include "sensors/sensors.h"

QueueHandle_t isrQueue;
BrainFrame brainFrame{};
DisplayEvent displayEvt;
TaskHandle_t readjustment_task_handle = nullptr;

void setMotorState(AppContext *app_ctx, MotorEvent evt)
{
    xQueueOverwrite(app_ctx->motorQueue, &evt);
}

void setDisplayState(AppContext *app_ctx, DisplayEvent evt)
{
    xQueueOverwrite(app_ctx->displayQueue, &evt);
}

int checkDeviation(uint32_t front, uint32_t side)
{
    return (side != NO_US_DIST) && (side < front) && (front - side >= DEVIATION_THRESHOLD);
}

void button_isr()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; // probably not necessary - button handler won't be higher priority than anything(?)

    uint8_t send = 0;
    xQueueSendFromISR(isrQueue, &send, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR(); // let the new high-priority task run now
}

void brainTask(void *pvParameters)
{
    AppContext *app_ctx = static_cast<AppContext *>(pvParameters);

    while (!app_ctx->displayQueue)
        vTaskDelay(pdMS_TO_TICKS(100)); // wait for other child task queues to come in

    xQueueReceive(app_ctx->brainQueue, &brainFrame, portMAX_DELAY);

    // wait for button press
    setDisplayState(app_ctx, CountDown1);

    uint8_t recv;
    isrQueue = xQueueCreate(1, sizeof(uint8_t));

    attachInterrupt(COUNTDOWN_BUTTON, button_isr, FALLING);
    xQueueReceive(isrQueue, &recv, portMAX_DELAY); // wait for button press

    // button pressed - countdown and flash to indicate
    for (int i = 1; i <= 3; i++)
    {
        setDisplayState(app_ctx, CountDown2);
        vTaskDelay(pdMS_TO_TICKS(500));

        setDisplayState(app_ctx, CountDown1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // got response from sensors -> ready to go
    setDisplayState(app_ctx, Start);

    // Note; don't use TurnCW - broken motor (screw loose)

    BotState botState = Stopped;
    MotorEvent motorEvt;

    TickType_t adjustmentTime;

    for (;;)
    {
        xQueueReceive(app_ctx->brainQueue, &brainFrame, portMAX_DELAY);
        // Serial.printf("Ultrasonics: Front left: %d, front centre %d, front right: %d\n", brainFrame.us.front_left_dist, brainFrame.us.front_centre_dist, brainFrame.us.front_right_dist);
        // Serial.printf("IRs: Front left: %d, front centre %d, front right: %d, rear left: %d, rear right: %d\n\n", brainFrame.ir.front_left, brainFrame.ir.front_centre, brainFrame.ir.front_right, brainFrame.ir.rear_left, brainFrame.ir.rear_right);
        // continue;
        // brain tings

        // Step 1 - check front IRs
        if (brainFrame.ir.front_right || brainFrame.ir.front_centre || brainFrame.ir.front_left)
        {
            // one of the front IR(s) has fired! We're at an edge, so stop everything else

            // Step 1.i - BRAAAAAKE!! Front face on line
            if (botState != LineTurning && botState != Stopped)
            {
                // line detected, but am not currently turning or stopped - so, first stop
                setMotorState(app_ctx, ActiveBrake);

                vTaskDelay(pdMS_TO_TICKS(ACTIVE_BRAKE_MS)); // yep, task won't do ANYTHING else until we have stopped - this is critical

                botState = Stopped;
                setDisplayState(app_ctx, DisplayStopped);
                continue; // reload to detect any updates (unlikely)
            }

            if (botState != LineTurning)
            {
                // Step 1.ii - Braked, now we turn

                // Ok bot is stopped, nest best step is to *always* turn ACW imo - pivot about the right wheel (can't really do anything else)
                setMotorState(app_ctx, TurnACW);

                botState = LineTurning;
                setDisplayState(app_ctx, DisplayTurning);
                continue;
            }

            // Ok, we're turning - be patient
        }

        // Step 2 - check rear IRs
        else if (brainFrame.ir.rear_left || brainFrame.ir.rear_right)
        {
            // realistically, we're only going to be in this state because we turned all the way around without finiding a target so that our rear has now gone over
            // this will *always* happen because pivoting about left wheel
            // nonetheless, at this point stop turning and go straight - same response regardless of whether we are here through some extraordinary reason, or simply because we turned

            // cool thing is we trace a star pattern because we always do 180 + delta turn (corner is on line)

            // Step 2.i - rears fired - brake first
            if (botState != Stopped)
            {
                setMotorState(app_ctx, ActiveBrake);

                vTaskDelay(pdMS_TO_TICKS(ACTIVE_BRAKE_MS));

                botState = Stopped;
                continue;
            }

            // Step 2.ii - go straight (away from boundry)
            setMotorState(app_ctx, DriveStraight);

            setDisplayState(app_ctx, DisplayStraight);
            botState = Straight;
        }

        // Step 3 - Check ultrasonics
        // no IRs are being fired -> we are either turning but outside the line, or moving straight
        else
        {
            if (brainFrame.us.front_centre_dist != NO_US_DIST && brainFrame.us.front_centre_dist <= TARGET_DISTANCE_THRESHOLD)
            {
                if (botState == LineTurning)
                {
                    // target detected while doing line turning - brake asap
                    setMotorState(app_ctx, ActiveBrake);
                    vTaskDelay(pdMS_TO_TICKS(ACTIVE_BRAKE_MS));

                    setDisplayState(app_ctx, DisplayStopped);
                    botState = Stopped;

                    continue;
                }

                // this also enables mid-path scanning
                if (checkDeviation(brainFrame.us.front_centre_dist, brainFrame.us.front_left_dist) || checkDeviation(brainFrame.us.front_centre_dist, brainFrame.us.front_right_dist))
                {
                    adjustmentTime = xTaskGetTickCount();
                    if (!ScanTurning)
                    {
                        // the front and side sensors vary too much, turn until not the case
                        setMotorState(app_ctx, TurnACW);
                        setDisplayState(app_ctx, DisplayReadjusting);
                        botState = ScanTurning;
                    }

                    continue;
                }

                // now, either target is right in front or max readjustment period elapsed (target disappeared) - go straight
                setMotorState(app_ctx, DriveStraight);

                setDisplayState(app_ctx, DisplayStraight);
                botState = Straight;

                // alright - we want to find shortest path, so all other functioning ultrasonics have to report a lower distance
            }
            else if (botState != ScanTurning || (pdTICKS_TO_MS(adjustmentTime - xTaskGetTickCount()) >= MAX_READJUSTMENT_TIME_MS))
            {
                // nothing to do, continue straight till we reach the other end!
                setMotorState(app_ctx, DriveStraight);

                setDisplayState(app_ctx, DisplayStraight);
                botState = Straight;
                adjustmentTime = 0;
            }
        }
    }

    vTaskDelete(NULL);
}