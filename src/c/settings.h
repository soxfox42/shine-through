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
    GColor palette[4];
    GColor text_color;
    TextMode top_text, bottom_text;
    bool enable_outlines;
    GColor outline_color;
} Settings;

extern Settings g_settings;

void settings_init(void);
