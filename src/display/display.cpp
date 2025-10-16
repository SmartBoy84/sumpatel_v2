#include <Arduino.h>
#include <TFT_eSPI.h>

#include "display.h"
#include "app_types.h"

void displayTask(void *pvParameters)
{
    AppContext *app_ctx = static_cast<AppContext *>(pvParameters);

    // initialisation
    TFT_eSPI *tft = app_ctx->tft;

    tft->init();
    tft->fillScreen(TFT_SKYBLUE);

    DisplayEvent evt;

    tft->fillScreen(TFT_BLACK); // off, until otherwise

    uint32_t colour;

    for (;;)
    {
        // block until want to update screen
        if (!xQueueReceive(app_ctx->displayQueue, &evt, portMAX_DELAY))
            continue;

        // indicate state
        switch (evt)
        {
        case SensorsInitialising:
            colour = TFT_YELLOW;
            break;

        case CountDown1:
            colour = TFT_LIGHTGREY;
            break;

        case CountDown2:
            colour = TFT_DARKGREY;
            break;

        case Start:
            colour = TFT_WHITE;
            break;

        // brain states
        case DisplayTurning:
            colour = TFT_BLUE;
            break;

        case DisplayStraight:
            colour = TFT_GREEN;
            break;

        case DisplayStopped:
            colour = TFT_RED;
            break;

        case DisplayReadjusting:
            colour = TFT_PURPLE;
            break;
        default:
            colour = TFT_RED;
        }

        tft->fillScreen(colour);
    }

    vTaskDelete(NULL);
}