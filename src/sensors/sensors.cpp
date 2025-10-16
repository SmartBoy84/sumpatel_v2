#include <Wire.h>
#include <Adafruit_MCP23008.h>

#include "sensors.h"
#include "app_types.h"

#define US_WAIT pdMS_TO_TICKS(ROUND_ROBIN_MS) * 3 // don't wait too long

void sensorsTask(void *pvParameters)
{
  AppContext *app_ctx = static_cast<AppContext *>(pvParameters);
  Adafruit_MCP23008 *mcp = app_ctx->mcp;

  TaskHandle_t ultrasonic_task_handle = nullptr;
  QueueHandle_t ultrasonicQueue = xQueueCreate(1, sizeof(UltrasonicDistanceFrame));

  BrainFrame brainFrame{};

  // indicate initialisation in progress
  DisplayEvent dispEvt = SensorsInitialising;
  xQueueSend(app_ctx->displayQueue, &dispEvt, 0);

  // initialise the expander
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(WireClk);

  while (!mcp->begin(MCP_ADDR))
    vTaskDelay(pdMS_TO_TICKS(100));

  // initialise IR sensors
  mcp->pinMode(IR_FL_MCP, INPUT);
  mcp->pinMode(IR_FR_MCP, INPUT);
  mcp->pinMode(IR_FC_MCP, INPUT);
  mcp->pinMode(IR_RL_MCP, INPUT);
  mcp->pinMode(IR_RR_MCP, INPUT);

  // ultrasonic polling on separate task
  xTaskCreatePinnedToCore(ultrasonicTask, "UltrasonicTask", 4096, ultrasonicQueue, 0, &ultrasonic_task_handle, SECOND_CORE);
  xQueueReceive(ultrasonicQueue, &brainFrame.us, portMAX_DELAY); // wait until it starts at least
  for (;;)
  {
    // update perception shizzle
    // get ultrasonic
    xQueueReceive(ultrasonicQueue, &brainFrame.us, US_WAIT); // just use old one if no response

    // get ir
    brainFrame.ir.front_centre = mcp->digitalRead(IR_FC_MCP);
    brainFrame.ir.front_right = mcp->digitalRead(IR_FR_MCP);
    brainFrame.ir.front_left = mcp->digitalRead(IR_FL_MCP);
    brainFrame.ir.rear_right = mcp->digitalRead(IR_RR_MCP);
    brainFrame.ir.rear_left = mcp->digitalRead(IR_RL_MCP);

    // send brain update
    xQueueOverwrite(app_ctx->brainQueue, &brainFrame);
  }

  vTaskDelete(ultrasonic_task_handle);
  vTaskDelete(NULL);
}