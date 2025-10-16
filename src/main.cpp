#include <Arduino.h>

// libraries
#include <TFT_eSPI.h>
#include <Wire.h>
#include <Adafruit_MCP23008.h>

// task interop
#include "app_types.h"

// esp tasks
#include "display/display.h"
#include "brain/brain.h"
#include "motor/motor.h"
#include "sensors/sensors.h"

static AppContext app_ctx;

static TFT_eSPI tft;
static Adafruit_MCP23008 mcp;

TaskHandle_t display_task_handle, brain_task_handle, motor_task_handle, sensors_task_handle = nullptr;

MainEvent evt;

#define WireClk 100000

#define PIN_POWER_ON 15

void setup()
{
  Serial.begin(115200);

  // enable power of batt
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  // initialise context
  app_ctx.tft = &tft;
  app_ctx.mcp = &mcp;

  app_ctx.mainQueue = xQueueCreate(mainQueue_n, sizeof(MainEvent));
  app_ctx.displayQueue = xQueueCreate(displayQueue_n, sizeof(DisplayEvent));
  app_ctx.brainQueue = xQueueCreate(brainQueue_n, sizeof(BrainFrame));
  app_ctx.motorQueue = xQueueCreate(motorQueue_n, sizeof(MotorEvent));

  // create tasks - make sure stack is decently big!

  // IMPORTANT - motorTask init first so the drive can be turned off ASAP, otherwise seg fault can cause them to fire
  xTaskCreatePinnedToCore(motorTask, "MotorTask", 2048, &app_ctx, 1, &motor_task_handle, APP_CPU_NUM);
  for (;;)
    if (xQueueReceive(app_ctx.mainQueue, &evt, portMAX_DELAY) && evt == MotorsInitialised)
    {
      yield(); // allow watchdog reset ig
      break;
    }

  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, &app_ctx, 1, &display_task_handle, APP_CPU_NUM);
  xTaskCreatePinnedToCore(brainTask, "BrainTask", 2048, &app_ctx, 1, &brain_task_handle, APP_CPU_NUM);
  xTaskCreatePinnedToCore(sensorsTask, "SensorsTask", 4096, &app_ctx, 1, &sensors_task_handle, APP_CPU_NUM);
}

void loop()
{
}