#include "sensors.h"
#include "app_types.h"

void init_us()
{
    pinMode(TRIG_F, OUTPUT);
    pinMode(TRIG_FL, OUTPUT);
    pinMode(TRIG_FR, OUTPUT);

    pinMode(ECHO_F, INPUT);
    pinMode(ECHO_FL, INPUT);
    pinMode(ECHO_FR, INPUT);
}

uint32_t get_distance(int echo_pin, int trig_pin)
{
    // trigger echo
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);

    // get echo length, and convert to distance
    return pulseIn(echo_pin, HIGH, 30000UL) / 58;
}

void ultrasonicTask(void *pvParameters)
{
    init_us();

    UltrasonicDistanceFrame distanceFrame{};
    QueueHandle_t sensorQueue = static_cast<QueueHandle_t>(pvParameters); // private us <-> sensor task channel

    TickType_t roundRobinDelay = pdMS_TO_TICKS(ROUND_ROBIN_MS);

    for (;;)
    {
        distanceFrame.front_centre_dist = get_distance(ECHO_F, TRIG_F);
        vTaskDelay(roundRobinDelay);

        distanceFrame.front_left_dist = get_distance(ECHO_FL, TRIG_FL);
        vTaskDelay(roundRobinDelay);

        distanceFrame.front_right_dist = get_distance(ECHO_FR, TRIG_FR);
        vTaskDelay(roundRobinDelay);

        xQueueOverwrite(sensorQueue, &distanceFrame);
    }

    vTaskDelete(NULL);
}