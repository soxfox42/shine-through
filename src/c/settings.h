#pragma once

#include <pebble.h>

#define SETTINGS_KEY 1

typedef enum {
    TextModeNone,
    TextModeWeekdayDay,
    TextModeDayMonth,
    TextModeBattery,
    TextModeAMPM,
    TextModeSteps,
} TextMode;

typedef struct {
#ifdef PBL_COLOR
    GColor palette[4];
    GColor text_color;
#else
    uint8_t dither[4];
#endif
    TextMode top_text, bottom_text;
    bool enable_outlines;
    GColor outline_color;
} Settings;

extern Settings g_settings;

void settings_init(void);
