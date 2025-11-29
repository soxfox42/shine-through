#pragma once

#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct {
    GColor palette[4];
    GColor text_color;
} Settings;

extern Settings g_settings;

void settings_init(void);
