#ifndef DISPLAY_H
#define DISPLAY_H

#include "app_types.h"

void initDisplayTask();
void displayTask(void *pvParameters);

static inline void display_center_text(AppContext *ctx, const char *s,
                                       uint16_t fg = TFT_BLACK, uint16_t bg = 0, uint8_t size = 2);

#endif